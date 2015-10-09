/*
 INS_Disassemble(ins).c_str() -> to dump the instruction with a string ( ex. jz 0x08045678 ) 
 THIS TOOL DUMPS THE BASIC BLOCK INSTRUCTIONS 
*/
#include <stdio.h>
#include "pin.H"
#include <iostream>
#include <string>


FILE * file;


// This function is called when the application exits
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
    PIN_ERROR("This Pintool prints the hexadecimal of all the instruction executed\n" 
              + KNOB_BASE::StringKnobSummary() + "\n");
    return -1;
}




// Pin calls this function every time a new instruction is encountered
void Trace(TRACE trace , void *v)
{
	for(BBL bbl= TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl)){

		fprintf(file, "----BEGIN BBL---\n");
		
		for( INS ins = BBL_InsHead(bbl); INS_Valid(ins) ; ins =INS_Next(ins)){
		   fprintf(file , "%s\n" , INS_Disassemble(ins).c_str());
		}
		
		fprintf(file, "----END BBL---\n");
	}
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char * argv[])
{

    file  = fopen("itrace.out", "w");
    
    // Initialize pin
	PIN_InitSymbols();
    if (PIN_Init(argc, argv)) return Usage();

    
	TRACE_AddInstrumentFunction(Trace,0);


    // Register Fini to be called when the application exits
    PIN_AddFiniFunction(Fini, 0);
    

	
    // Start the program, never returns
    PIN_StartProgram();
    
    return 0;
}
