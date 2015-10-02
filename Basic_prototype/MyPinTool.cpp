#include <stdio.h>
#include "pin.H"
#include <stdlib.h>



FILE * file;

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{
	static int detected = 0; 

	int jmp_offset = 0x1600;

	if(INS_IsBranch(ins)){
		//get the address of the current instruction
		ADDRINT address = INS_Address(ins);
		//get the string rapresentation of the address
		//string addr = StringFromAddrint(address);
		//fprintf(file , "%s\n" , addr.c_str());

		//if we reach the address of the conditional jump (discover by hand with immunity)
		//we substitute this instruction with an uncoditional jump in order o force the execution path
		if(address == 0x00411970){
			//get the jmp target address
			ADDRINT tgt = INS_DirectBranchOrCallTargetAddress(ins);

			//insert the incoditional jmp
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
				   
					fprintf(file ,"[EOP TROVATO ] -> CEIP= %08x | TEIP= %08x | OFFSET = %08x\n" ,  current_eip , target_eip , address_delta);
					detected = 1;
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
    
    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    // Register Instruction to be called to instrument instructions
    INS_AddInstrumentFunction(Instruction, 0);

    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
