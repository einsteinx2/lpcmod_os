#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iomanip>
#include <stdlib.h>

#define SMBUS         0x0000c000

using namespace std;

string removeExtension(const std::string &filename) {
	size_t lastdot = filename.find_last_of(".");
	if (lastdot == std::string::npos)
		return filename;
	return filename.substr(0, lastdot);
}

int main(int argc, char* argv[]) {
	unsigned char buffer[0xBAC];
	int count = 0;
	int inSize;
	int tempVar, numOp1, numOp2;
	unsigned int valPosition;
	string inFilename;
	string xcodeString;
	string outLine5, outLine4, outLine3, outLine2, outLine1, outLine0,
			tempString;
	char xcodeOp1[50], xcodeOp2[50], tempUnkown[3];
	int op1Print = 0, op2Print = 0, ignore = 0, ignoreCount = 0;

	struct xcodeStruct {
		unsigned char opcode;
		unsigned int op1;
		unsigned int op2;
	}*xcode;

	if (argc != 2) {
		cout << "Specify filename of BIOS to decode.\nusage: " << argv[0]
				<< " <filename>\n";
	} else {
		inFilename = argv[1];

		cout << "Looking for file named \"" << inFilename << "\" as input.\n";
		ifstream inFile(inFilename.c_str(), ios::in | ios::binary | ios::ate);
		if (inFile.is_open()) {
			cout << "BIOS opened, so far so good!\n";
			inSize = (int) inFile.tellg();
			inFile.seekg(0, ios::beg);
			if (inSize != 262144) {
				cout
						<< "Original BIOS size mismatch!!\nYou do not have a proper BIOS image\n";
				return -1;
			}
		} else {
			cout << "Error opening original BIOS file, aborting.\n";
			return -1;
		}

		outLine5 = "";
		outLine4 = "";
		outLine3 = "";
		outLine2 = "";
		outLine1 = "";
		outLine0 = "";

		string outString = removeExtension(inFilename) + "_xcode.txt";
		ofstream outFile(outString.c_str(), ios::out);

		if (outFile.is_open()) {
			cout
					<< "Output text file created.\nParsing of the BIOS X-code region will now begin.\n";
		} else {
			cout << "Could not create the text file, aborting.\n";
			return -1;
		}
		inFile.seekg(0x80, ios::beg);
		inFile.read((char *) buffer, 0xBAC);

		for (count = 0; count < 0xBAC; count += 9) {
			xcode = (xcodeStruct *) (buffer + count);
			ignore = 0;
			sprintf(xcodeOp1, "%.2x%.2x%.2x%.2x", buffer[count + 4],
					buffer[count + 3], buffer[count + 2], buffer[count + 1]);
			sprintf(xcodeOp2, "%.2x%.2x%.2x%.2x", buffer[count + 8],
					buffer[count + 7], buffer[count + 6], buffer[count + 5]);
			numOp1 = strtoul(xcodeOp1, NULL, 16);
			numOp2 = strtoul(xcodeOp2, NULL, 16);
			sprintf(xcodeOp1, "0x%.2x%.2x%.2x%.2x", buffer[count + 4],
					buffer[count + 3], buffer[count + 2], buffer[count + 1]);
			sprintf(xcodeOp2, "0x%.2x%.2x%.2x%.2x", buffer[count + 8],
					buffer[count + 7], buffer[count + 6], buffer[count + 5]);

			switch (xcode->opcode) {
			case 0x0:
				xcodeString = "";
				op1Print = 0;
				op2Print = 0;
				ignore = 1;
				ignoreCount++;
				break;
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
				switch (numOp1) {
				case 0x3: //0x10100000:	//0x3
					xcodeString = "xcode_poke_a";
					break;
				case 0x4: //0x86000:		//0x4
					xcodeString = "xcode_pciout_a";
					break;
				case 0x11: //0x8b400:		//0x11
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
				xcodeString = "!!err_value=0x";
				sprintf(tempUnkown, "%.2x", xcode->opcode);
				xcodeString.append(tempUnkown);
				op1Print = 1;
				op2Print = 1;
				break;
			}

			if (!xcodeString.compare("xcode_ifgoto")) {
				tempVar = numOp2;
				tempVar = (tempVar / 9);
				tempVar += 1;
				sprintf(xcodeOp2, "0x%.8x", tempVar);
			}
			if (!xcodeString.compare("xcode_goto")) {
				tempVar = numOp2;
				tempVar = (tempVar / 9);
				tempVar += 1;
				sprintf(xcodeOp2, "0x%.8x", tempVar);
			}

			if (!xcodeString.compare("xcode_outb")
					|| !xcodeString.compare("xcode_inb")) {	//Check for SMBUS string swap
				if (numOp1 >= SMBUS && numOp1 <= (SMBUS + 0xFF)) {//In proper range.
					tempVar = numOp1 - SMBUS;
					if (tempVar)
						sprintf(xcodeOp1, "SMBUS+%u", tempVar);
					else
						sprintf(xcodeOp1, "SMBUS");
				}
			}

			if (!ignore && ignoreCount < 6) {
				outLine5 = xcodeString;
				if (op1Print == 1 && op2Print == 0) {
					outLine5 = xcodeString;
					outLine5.append("(");
					outLine5.append(xcodeOp1);
					outLine5.append(");");
				} else if (op1Print == 0 && op2Print == 1) {
					outLine5 = xcodeString;
					outLine5.append("(");
					outLine5.append(xcodeOp2);
					outLine5.append(");");
				} else if (op1Print == 1 && op2Print == 1) {
					//Both ops are to be written.
					outLine5 = xcodeString;
					outLine5.append("(");
					outLine5.append(xcodeOp1);
					outLine5.append(", ");
					outLine5.append(xcodeOp2);
					outLine5.append(");");
				}
			}

			if (!outLine5.compare("xcode_outb(SMBUS, 0x00000010);")
					&& !outLine4.compare("xcode_ifgoto(0x00000010, 0xffffffff);")
					&& !outLine3.compare("xcode_inb(SMBUS);")
					&& !outLine2.compare("xcode_outb(SMBUS+2, 0x0000000a);")
					&& outLine1.find("xcode_outb(SMBUS+6, ") != std::string::npos
					&& outLine0.find("xcode_outb(SMBUS+8, ") != std::string::npos) {
				outLine5 = "SMB_xcode_Write(";
				valPosition = outLine0.find("8, ");
				if (valPosition != std::string::npos)
					tempString = outLine0.substr(valPosition + 3, 10);
				outLine5.append(tempString);
				outLine5.append(", ");
				valPosition = outLine1.find("6, ");
				if (valPosition != std::string::npos)
					tempString = outLine1.substr(valPosition + 3, 12);
				outLine5.append(tempString);
				outLine0 = outLine5;
				outLine5 = "";
				outLine4 = "";
				outLine3 = "";
				outLine2 = "";
				outLine1 = "";

			}

			if (!outLine0.empty()) {
				outFile << outLine0 << std::endl;
			}

			outLine0 = outLine1;
			outLine1 = outLine2;
			outLine2 = outLine3;
			outLine3 = outLine4;
			outLine4 = outLine5;
			outLine5 = "";

		}
		if (!outLine1.empty()) {
			outFile << outLine1 << std::endl;
		}
		if (!outLine2.empty()) {
			outFile << outLine2 << std::endl;
		}
		if (!outLine3.empty()) {
			outFile << outLine3 << std::endl;
		}
		if (!outLine4.empty()) {
			outFile << outLine4 << std::endl;
		}
		if (!outLine5.empty()) {
			outFile << outLine5 << std::endl;
		}
		cout << "\"" << outString << "\" created.\nEnjoy!\n";
		outFile.close();
		inFile.close();
	}
	return 0;
}
