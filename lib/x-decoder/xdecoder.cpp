#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>

using namespace std;

string removeExtension(const std::string &filename) {
    size_t lastdot = filename.find_last_of(".");
    if (lastdot == std::string::npos) return filename;
    return filename.substr(0, lastdot);
}


int main(int argc, char* argv[])
{
  char buffer[0xBAC];
  int count = 0;
  int inSize;
  string inFilename;
  string xcodeString;
  stringstream xcodeOp1, xcodeOp2;
  int op1Print = 0, op2Print = 0;

  struct xcodeStruct{
    unsigned char opcode;
    unsigned int op1;
    unsigned int op2;
  } *xcode;
  if (argc != 2){
    cout<<"Specify filename of BIOS to decode.\nusage: "<< argv[0] <<" <filename>\n";
  }
  else {
	inFilename = argv[1];

	cout << "Looking for file named \"" << inFilename << "\" as input.\n";
	ifstream inFile(inFilename.c_str(), ios::in|ios::binary|ios::ate);
	if(inFile.is_open()){
		cout << "BIOS opened, so far so good!\n";
		inSize = (int) inFile.tellg();
		inFile.seekg (0, ios::beg);
		if(inSize != 262144){
		  cout << "Original BIOS size mismatch!!\nYou do not have a proper BIOS image\n";
			return -1;
		}
	}
	else {
		cout << "Error opening original BIOS file, aborting.\n";
		return -1;
	}

	string outString = removeExtension(inFilename) + "_xcode.txt";
	ofstream outFile (outString.c_str(), ios::out);

	if(outFile.is_open()){
		cout << "Output text file created.\nParsing of the BIOS X-code region will now begin.\n";
	}
	else{
		cout << "Could not create the text file, aborting.\n";
		return -1;
	}
	inFile.seekg (0x80, ios::beg);
	inFile.read(buffer, 0xBAC);
	for(count = 0; count < 0xBAC; count+=9){
	xcodeOp1.clear();
	xcodeOp1.str(string());
	xcodeOp2.clear();
	xcodeOp2.str(string());
	xcode = (xcodeStruct *)(buffer + count);
	switch(xcode->opcode) {
        case 0x2:
            xcodeString = "xcode_peek";
            op1Print = 1;
            op2Print = 0;
            break;
        case 0x3:
            xcodeString = "xcode_poke";
            op1Print = 1;
            op2Print = 1;
            break;
        case 0x4:
            xcodeString = "xcode_pciout";
            op1Print = 1;
            op2Print = 1;
            break;
        case 0x5:
            xcodeString = "xcode_pciin_a";
            op1Print = 1;
            op2Print = 0;
            break;
        case 0x6:
            xcodeString = "xcode_bittoggle";
            op1Print = 1;
            op2Print = 1;
            break;
        case 0x7:
            switch(xcode->op1){
            case 0x3:
            	xcodeString = "xcode_poke_a";
            	break;
            case 0x4:
            	xcodeString = "xcode_pciout_a";
            	break;
            case 0x11:
            	xcodeString = "xcode_outb_a";
            	break;
            default:
            	xcodeString = "err0x7!!!!";
            	break;
            }
            op1Print = 0;
            op2Print = 1;
            break;
        case 0x8:
            xcodeString = "xcode_ifgoto";
            op1Print = 1;
            op2Print = 1;
            break;
        case 0x9:
            xcodeString = "xcode_goto";
            op1Print = 0;
            op2Print = 1;
            break;
        case 0x10:
            xcodeString = "_____unused";
            op1Print = 0;
            op2Print = 0;
            break;
        case 0x11:
            xcodeString = "xcode_outb";
            op1Print = 1;
            op2Print = 1;
            break;
        case 0x12:
            xcodeString = "xcode_inb";
            op1Print = 1;
            op2Print = 0;
            break;
        case 0xEE:
            xcodeString = "xcode_END";
            op1Print = 1;
            op2Print = 0;
            break;
        default:
            xcodeString = "err!!!!!!";
            op1Print = 0;
            op2Print = 0;
            break;
    	}
    	xcodeOp1 << setfill('0') << setw(8) << hex << xcode->op1;
    	xcodeOp2 << setfill('0') << setw(8) << hex << xcode->op2;
    	if(op1Print == 0 && op2Print == 0)
		outFile << xcodeString << std::endl;
	if(op1Print == 1 && op2Print == 0)
		outFile << xcodeString << "(0x" << xcodeOp1.str() << ");" << std::endl;
	if(op1Print == 0 && op2Print == 1)
		outFile << xcodeString << "(0x" << xcodeOp2.str() << ");" << std::endl;
	else	//Both ops are to be written.
		outFile << xcodeString << "(0x" << xcodeOp1.str() <<", 0x" << xcodeOp2.str() << ");" << std::endl;

	}
	cout << "\"" << outString << "\" created.\nEnjoy!\n";
	outFile.close();
	inFile.close();
  }
return 0;
}

