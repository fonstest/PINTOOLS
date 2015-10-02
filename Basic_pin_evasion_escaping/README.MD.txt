## Basic Pin evasion escaping

Running pin with this program (pin\_escaping\_detect_parent), pin will be detected and you will see **'Detected!!'**.
Otherwise if you use pin with our pintool you will see **'Not Detected!!'**.

**This pin tool is very simple and works only with the specified program (pin\_escaping\_detect_parent)!!!**

### Workflow

1. Opening the program in IDA, searching for the string 'Detected!!', you can see the code point in which the evasion takes place.
![screen1](./screenshot/screen1.png)

2. When the pintool detects the address of the evasion jump, the instruction is deleted and substituted with an incoditional jump
to the **'Not Detected!!'** branch.  
![screen2](./screenshot/screen2.png)