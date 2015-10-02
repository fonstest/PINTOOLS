#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include "pin.H"




FILE * file;


static void ConnectDebugger()
{
    if (PIN_GetDebugStatus() != DEBUG_STATUS_UNCONNECTED){
		 fprintf(file, "errore  1");
		 return;
	}


    DEBUG_CONNECTION_INFO info;
    if (!PIN_GetDebugConnectionInfo(&info) || info._type != DEBUG_CONNECTION_TYPE_TCP_SERVER){
		 fprintf(file, "error 2");
		 return;
	}


    fprintf(file, "uscito");

	int timeout = 30000;

    if (PIN_WaitForDebuggerToConnect(timeout))
        return;

}


static VOID DoBreakpoint(const CONTEXT *ctxt, THREADID tid)
{
	
    ConnectDebugger();  // Ask the user to connect a debugger, if it is not already connected.

    // Construct a string that the debugger will print when it stops.  If a debugger is
    // not connected, no breakpoint is triggered and execution resumes immediately.

    PIN_ApplicationBreakpoint(ctxt, tid, FALSE, "DEBUGGER");
}

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{
	static int detected = 0; 

	int jmp_offset = 0x1600;

	if(INS_IsBranch(ins)){
		//get the address of the current instruction
		ADDRINT address = INS_Address(ins);

		//if we reach the address of the conditional jump (discover by hand with immunity)
		//we substitute this instruction with an uncoditional jump in order o force the execution path
		//this check is triggered only after the OEP is been found
		if(address == 0x00411a10 && detected == 1){
			//get the jmp target address
			ADDRINT tgt = INS_DirectBranchOrCallTargetAddress(ins);

			//insert the uncoditional jmp
			INS_InsertDirectJump(ins, IPOINT_BEFORE, tgt); 

			//remove the conditional jmp instruction
			INS_Delete(ins);
		}

		else {  //let's trigger our simple heuristic about the oep 
		
			if(INS_IsDirectBranchOrCall(ins) && address >= 0x401000 && address <= 0x41e000 && detected == 0 )
			  {
				ADDRINT current_eip = INS_Address(ins); //eip of the current instruction 
				ADDRINT target_eip = INS_DirectBranchOrCallTargetAddress(ins);

				int current_eip_signed = (int) current_eip;
				int target_eip_signed = (int) target_eip;

				int address_delta = abs(target_eip_signed - current_eip_signed);

				// let's block at the first long jump in the address space range
				// of the section of the program [ BRUTAL HEURISTIC THAT WORKS ONLY WITH 1 LAYER PACKERS ]
				if(  address_delta >= jmp_offset){
				   
					fprintf(file ,"[OEP TROVATO ] -> CEIP= %08x | TEIP= %08x | OFFSET = %08x\n" ,  current_eip , target_eip , address_delta);
					detected = 1;
					ConnectDebugger();
					INS_InsertCall(ins,  IPOINT_BEFORE, (AFUNPTR)DoBreakpoint, IARG_CONST_CONTEXT, IARG_THREAD_ID, IARG_END);
				}

			}
		}
	}
	

}



VOID Fini(INT32 code, VOID *v)
{
    fclose(file);
}

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    PIN_ERROR("This Pintool prints the IPs of every instruction executed\n" 
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    file = fopen("itrace.out", "w");

	//debug options
	DEBUG_MODE mode;
	_tcpClientStruct client;

	client._ip = "127.0.0.1";
	client._tcpPort = 8888;

    mode._type = DEBUG_CONNECTION_TYPE_TCP_CLIENT;
	mode._options = DEBUG_MODE_OPTION_STOP_AT_ENTRY;
    mode._tcpClient = client;
    
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);

	PIN_SetDebugMode(&mode);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
