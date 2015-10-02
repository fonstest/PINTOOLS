#include <stdio.h>
#include "pin.H"

FILE * file;

// Pin calls this function every time a new instruction is encountered
VOID Instruction(INS ins, VOID *v)
{
	//get the address of the current instruction
	ADDRINT address = INS_Address(ins);
	//get the string rapresentation of the address
	string addr = StringFromAddrint(address);
	//if we reach the address of the conditional jump (discover by hand with immunity)
	//we substitute this instruction with an uncoditional jump in order o force the execution path
	if(addr.compare("0x00411a10") == 0){
		//get the jmp target address
		ADDRINT tgt = INS_DirectBranchOrCallTargetAddress(ins);
		string addr2 = StringFromAddrint(tgt);
		//insert the incoditional jmp
		INS_InsertDirectJump(ins, IPOINT_BEFORE, tgt); 
		//fprintf(file, "%s\n", addr2.c_str()); 
		//remove the conditional jmp instruction
		INS_Delete(ins);
	}
	

}



VOID Fini(INT32 code, VOID *v)
{
    fprintf(file, "#eof\n");
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
