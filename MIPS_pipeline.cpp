#include<iostream>
#include<string>
#include<vector>
#include<bitset>
#include<fstream>
using namespace std;
#define MemSize 1000 // memory size, in reality, the memory size should be 2^32, but for this lab, for the space resaon, we keep it as this large number, but the memory is still 32-bit addressable.

struct IFStruct {
    bitset<32>  PC;
    bool        nop;  
};

struct IDStruct {
    bitset<32>  Instr;
    bool        nop;  
};

struct EXStruct {
    bitset<32>  Read_data1;
    bitset<32>  Read_data2;
    bitset<16>  Imm;
    bitset<5>   Rs;
    bitset<5>   Rt;
    bitset<5>   Wrt_reg_addr;
    bool        is_I_type;
    bool        rd_mem;
    bool        wrt_mem; 
    bool        alu_op;     //1 for addu, lw, sw, 0 for subu 
    bool        wrt_enable;
    bool        nop;  
};

struct MEMStruct {
    bitset<32>  ALUresult;
    bitset<32>  Store_data;
    bitset<5>   Rs;
    bitset<5>   Rt;    
    bitset<5>   Wrt_reg_addr; //Rd
    bool        rd_mem;
    bool        wrt_mem; 
    bool        wrt_enable;    
    bool        nop;    
};

struct WBStruct {
    bitset<32>  Wrt_data;
    bitset<5>   Rs;
    bitset<5>   Rt;     
    bitset<5>   Wrt_reg_addr;
    bool        wrt_enable;
    bool        nop;     
};

struct stateStruct {
    IFStruct    IF;
    IDStruct    ID;
    EXStruct    EX;
    MEMStruct   MEM;
    WBStruct    WB;
};

class RF
{
    public: 
        bitset<32> Reg_data;
     	RF()
    	{ 
			Registers.resize(32);  
			Registers[0] = bitset<32> (0);  
        }
	
        bitset<32> readRF(bitset<5> Reg_addr)
        {   
            Reg_data = Registers[Reg_addr.to_ulong()];
            return Reg_data;
        }
    
        void writeRF(bitset<5> Reg_addr, bitset<32> Wrt_reg_data)
        {
            Registers[Reg_addr.to_ulong()] = Wrt_reg_data;
        }
		 
		void outputRF()
		{
			ofstream rfout;
			rfout.open("RFresult.txt",std::ios_base::app);
			if (rfout.is_open())
			{
				rfout<<"State of RF:\t"<<endl;
				for (int j = 0; j<32; j++)
				{        
					rfout << Registers[j]<<endl;
				}
			}
			else cout<<"Unable to open file";
			rfout.close();               
		} 
			
	private:
		vector<bitset<32> >Registers;	
};

class INSMem
{
	public:
        bitset<32> Instruction;
        INSMem()
        {       
			IMem.resize(MemSize); 
            ifstream imem;
			string line;
			int i=0;
			imem.open("imem.txt");
			if (imem.is_open())
			{
				while (getline(imem,line))
				{      
					IMem[i] = bitset<8>(line);
					i++;
				}                    
			}
            else cout<<"Unable to open file";
			imem.close();                     
		}
                  
		bitset<32> readInstr(bitset<32> ReadAddress) 
		{    
			string insmem;
			insmem.append(IMem[ReadAddress.to_ulong()].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+1].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+2].to_string());
			insmem.append(IMem[ReadAddress.to_ulong()+3].to_string());
			Instruction = bitset<32>(insmem);		//read instruction memory
			return Instruction;     
		}     
      
    private:
        vector<bitset<8> > IMem;     
};
      
class DataMem    
{
    public:
        bitset<32> ReadData;  
        DataMem()
        {
            DMem.resize(MemSize); 
            ifstream dmem;
            string line;
            int i=0;
            dmem.open("dmem.txt");
            if (dmem.is_open())
            {
                while (getline(dmem,line))
                {      
                    DMem[i] = bitset<8>(line);
                    i++;
                }
            }
            else cout<<"Unable to open file";
                dmem.close();          
        }
		
        bitset<32> readDataMem(bitset<32> Address)
        {	
			string datamem;
            datamem.append(DMem[Address.to_ulong()].to_string());
            datamem.append(DMem[Address.to_ulong()+1].to_string());
            datamem.append(DMem[Address.to_ulong()+2].to_string());
            datamem.append(DMem[Address.to_ulong()+3].to_string());
            ReadData = bitset<32>(datamem);		//read data memory
            return ReadData;               
		}
            
        void writeDataMem(bitset<32> Address, bitset<32> WriteData)            
        {
            DMem[Address.to_ulong()] = bitset<8>(WriteData.to_string().substr(0,8));
            DMem[Address.to_ulong()+1] = bitset<8>(WriteData.to_string().substr(8,8));
            DMem[Address.to_ulong()+2] = bitset<8>(WriteData.to_string().substr(16,8));
            DMem[Address.to_ulong()+3] = bitset<8>(WriteData.to_string().substr(24,8));  
        }   
                     
        void outputDataMem()
        {
            ofstream dmemout;
            dmemout.open("dmemresult.txt");
            if (dmemout.is_open())
            {
                for (int j = 0; j< 1000; j++)
                {     
                    dmemout << DMem[j]<<endl;
                }
                     
            }
            else cout<<"Unable to open file";
            dmemout.close();               
        }             
      
    private:
		vector<bitset<8> > DMem;      
};  

void printState(stateStruct state, int cycle)
{
    ofstream printstate;
    printstate.open("stateresult.txt", std::ios_base::app);
    if (printstate.is_open())
    {
        printstate<<"State after executing cycle:\t"<<cycle<<endl; 
        
        printstate<<"IF.PC:\t"<<state.IF.PC.to_ulong()<<endl;        
        printstate<<"IF.nop:\t"<<state.IF.nop<<endl; 
        
        printstate<<"ID.Instr:\t"<<state.ID.Instr<<endl; 
        printstate<<"ID.nop:\t"<<state.ID.nop<<endl;
        
        printstate<<"EX.Read_data1:\t"<<state.EX.Read_data1<<endl;
        printstate<<"EX.Read_data2:\t"<<state.EX.Read_data2<<endl;
        printstate<<"EX.Imm:\t"<<state.EX.Imm<<endl; 
        printstate<<"EX.Rs:\t"<<state.EX.Rs<<endl;
        printstate<<"EX.Rt:\t"<<state.EX.Rt<<endl;
        printstate<<"EX.Wrt_reg_addr:\t"<<state.EX.Wrt_reg_addr<<endl;
        printstate<<"EX.is_I_type:\t"<<state.EX.is_I_type<<endl; 
        printstate<<"EX.rd_mem:\t"<<state.EX.rd_mem<<endl;
        printstate<<"EX.wrt_mem:\t"<<state.EX.wrt_mem<<endl;        
        printstate<<"EX.alu_op:\t"<<state.EX.alu_op<<endl;
        printstate<<"EX.wrt_enable:\t"<<state.EX.wrt_enable<<endl;
        printstate<<"EX.nop:\t"<<state.EX.nop<<endl;        

        printstate<<"MEM.ALUresult:\t"<<state.MEM.ALUresult<<endl;
        printstate<<"MEM.Store_data:\t"<<state.MEM.Store_data<<endl; 
        printstate<<"MEM.Rs:\t"<<state.MEM.Rs<<endl;
        printstate<<"MEM.Rt:\t"<<state.MEM.Rt<<endl;   
        printstate<<"MEM.Wrt_reg_addr:\t"<<state.MEM.Wrt_reg_addr<<endl;              
        printstate<<"MEM.rd_mem:\t"<<state.MEM.rd_mem<<endl;
        printstate<<"MEM.wrt_mem:\t"<<state.MEM.wrt_mem<<endl; 
        printstate<<"MEM.wrt_enable:\t"<<state.MEM.wrt_enable<<endl;         
        printstate<<"MEM.nop:\t"<<state.MEM.nop<<endl;        

        printstate<<"WB.Wrt_data:\t"<<state.WB.Wrt_data<<endl;
        printstate<<"WB.Rs:\t"<<state.WB.Rs<<endl;
        printstate<<"WB.Rt:\t"<<state.WB.Rt<<endl;        
        printstate<<"WB.Wrt_reg_addr:\t"<<state.WB.Wrt_reg_addr<<endl;
        printstate<<"WB.wrt_enable:\t"<<state.WB.wrt_enable<<endl;        
        printstate<<"WB.nop:\t"<<state.WB.nop<<endl; 
    }
    else cout<<"Unable to open file";
    printstate.close();
}
 
//imem-original
//100011 00000 00001 0000000000000000 lw $1, $0(0) $1=11111111111111111111111111111111
//100011 00000 00010 0000000000000100 lw $2, $0(4) $2=01111111111111111111111111111110
//100011 00000 00011 0000000000000000 lw $3, $0(0) $3=11111111111111111111111111111111
//100011 00000 00100 0000000000000100 lw $4, $0(4) $4=01111111111111111111111111111110
//100011 00000 00101 0000000000000000 lw $5, $0(0) $5=11111111111111111111111111111111
//000000 00001 00010 00110 00000 100001 addu $1, $2, $6 $6=01111111111111111111111111111110
//000000 00001 00010 00111 00000 100011 subu $1, $2, $7 $7=10000001000000000000000000000001
//101011 00000 00001 0000000000001000 sw $1, $0(8)
//11111111111111111111111111111111 halt

//imem-RAW
//100011 00000 00001 0000000000001000 lw $1, $0(8)
//100011 00000 00010 0000000000000100 lw $2, $0(4) $2=01111111111111111111111111111110
//100011 00000 00011 0000000000000000 lw $3, $0(0) $3=11111111111111111111111111111111
//000000 00001 00001 00001 00000 100001 addu $1, $1, $1
//000000 00001 00001 00001 00000 100001 addu $1, $1, $1

//imem-beq do not branch
//100011 00000 00001 0000000000001000 lw $1, $0(8) 
//100011 00000 00010 0000000000001000 lw $2, $0(8) 
//000100 00001 00010 0000000000000100 bne $1, $2, 4 (beq in this lab)                    
//000000 00001 00001 00001 00000 100001 addu $1, $1, $1 

//imem-beq do branch
//100011 00000 00001 0000000000001000 lw $1, $0(8) 
//100011 00000 00010 0000000000001100 lw $2, $0(12)
//100011 00000 00011 0000000000000000 lw $3, $0(0) do nothing to avoid load-use
//100011 00000 00011 0000000000000000 lw $3, $0(0) do nothing to avoid load-use
//100011 00000 00011 0000000000000000 lw $3, $0(0) do nothing to avoid load-use
//000100 00001 00010 0000000000000100 bne $1, $2, 4 (beq in this lab)                    
//000000 00001 00001 00001 00000 100001 addu $1, $1, $1 

//test2
//100011 00000 00001 0000000000000000 lw $1, $0(0)
//100011 00000 00010 0000000000000100 lw $2, $0(4)
//100011 00000 00011 0000000000000000 lw $3, $0(0)
//100011 00000 00100 0000000000000100 lw $4, $0(4)
//100011 00000 00101 0000000000000000 lw $5, $0(0)
//000000 00001 00010 00110 00000 100001 addu $1, $2, $6  1+2
//000000 00110 00010 00100 00000 100011 subu $6, $2, $4  3-2
//101011 00000 00100 0000000000001000 sw $4, $0(8) 

//test3
//100011 00000000010000000000000000
//100011 00000000100000000000000100
//100011 00000000110000000000000000
//100011 00000001000000000000000100
//100011 00000001010000000000000000
//000000 00001000100011000000100001
//000000 00110000100010000000100011
//000100 00101000010000000000000010
//101011 00000001000000000000001000
//11111111111111111111111111111111
//000000 00001000100010000000100001
//101011 00000001000000000000001000

bitset<32> get_Branch_Addr(bitset<16> Imm){
    bitset<32> result=0;
    if(Imm[15]==true){
        //sign-extend 1
        result=bitset<32>("1111111111111111"+Imm.to_string());
    }else{
        result=bitset<32>(Imm.to_string());
    }
    result=result<<2;
    return result;
}


int main()
{
    
    RF myRF;
    INSMem myInsMem;
    DataMem myDataMem;
    stateStruct state{0};
    state.IF.nop = false;
    state.ID.nop = true;
    state.EX.nop = true;
    state.MEM.nop = true;
    state.WB.nop = true;
    state.EX.alu_op = true;
    int cycle = 0;

    bool do_Branch=false;

    //remove("dmemresult.txt");
    //remove("RFresult.txt");
    //remove("stateresult.txt");

             
    while (1) {
        stateStruct newState{0};

        /* --------------------- WB stage --------------------- */
        if(state.WB.nop==false){

            //forward RAW MEM unit
            if(state.EX.is_I_type==false && state.EX.wrt_enable==true){
                if(state.WB.Wrt_reg_addr!=bitset<5>("00000")){
                    if(state.WB.Wrt_reg_addr==state.EX.Rs){
                        state.EX.Read_data1=state.WB.Wrt_data;
                    }
                    if(state.WB.Wrt_reg_addr==state.EX.Rt){
                        state.EX.Read_data2=state.WB.Wrt_data;
                    }
                }
            }

            if(state.WB.wrt_enable==true){
                myRF.writeRF(state.WB.Wrt_reg_addr,state.WB.Wrt_data);
                //cout<<"update "<<state.WB.Wrt_reg_addr<<" with "<<state.WB.Wrt_data<<endl;
            }
        }

        /* --------------------- MEM stage --------------------- */
        if(state.MEM.nop==false){

            //forward RAW EX  unit
            if(state.EX.is_I_type==false && state.EX.wrt_enable==true){
                if(state.MEM.Wrt_reg_addr!=bitset<5>("00000")){
                    if(state.MEM.Wrt_reg_addr==state.EX.Rs){
                        state.EX.Read_data1=state.MEM.ALUresult;
                    }
                    if(state.MEM.Wrt_reg_addr==state.EX.Rt){
                        state.EX.Read_data2=state.MEM.ALUresult;
                    }
                }
            }




            if(state.MEM.rd_mem==false && state.MEM.wrt_mem==false){
                //R-type instruction, do nothing
                newState.WB.Wrt_data=state.MEM.ALUresult;
            }else if(state.MEM.rd_mem==true && state.MEM.wrt_mem==false){
                //lw instruction, read from memory
                newState.WB.Wrt_data=myDataMem.readDataMem(state.MEM.ALUresult);
            }else if(state.MEM.rd_mem==false && state.MEM.wrt_mem==true){
                //sw intrusion, write to memory
                myDataMem.writeDataMem(state.MEM.ALUresult,state.MEM.Store_data);
            }

            //update newState for next WB
            newState.WB.nop=state.MEM.nop;
            newState.WB.wrt_enable=state.MEM.wrt_enable;
            newState.WB.Rs=state.MEM.Rs;
            newState.WB.Rt=state.MEM.Rt;
            newState.WB.Wrt_reg_addr=state.MEM.Wrt_reg_addr;
            
        }else{
            newState.WB.nop=true;
        }
        

        
        /* --------------------- EX stage --------------------- */
        if(state.EX.nop==false){
            if(state.EX.alu_op==0 && state.EX.is_I_type==false){
                //subu instruction
                newState.MEM.ALUresult=bitset<32>(state.EX.Read_data1.to_ulong()-state.EX.Read_data2.to_ulong());
            }else if(state.EX.alu_op==1 && state.EX.is_I_type==false){
                //addu instruction
                newState.MEM.ALUresult=bitset<32>(state.EX.Read_data1.to_ulong()+state.EX.Read_data2.to_ulong());

            }else if(state.EX.alu_op==1 && state.EX.is_I_type==true){
                if(state.EX.wrt_mem==true){
                    //sw instruction only
                    newState.MEM.Store_data=state.EX.Read_data2;
                }

                //lw & sw instruction
                newState.MEM.ALUresult=bitset<32>(state.EX.Imm.to_ulong()+state.EX.Read_data1.to_ulong());
                
            }

            //update newState for next MEM
            newState.MEM.nop=state.EX.nop;
            newState.MEM.wrt_enable=state.EX.wrt_enable;
            newState.MEM.Rs=state.EX.Rs;
            newState.MEM.Rt=state.EX.Rt;
            newState.MEM.Wrt_reg_addr=state.EX.Wrt_reg_addr;
            newState.MEM.rd_mem=state.EX.rd_mem;
            newState.MEM.wrt_mem=state.EX.wrt_mem;
            
        }else{
            newState.MEM.nop=true;
        }

        /* --------------------- ID stage --------------------- */
        if(state.ID.nop==false){

            //for next EX 
            switch(stoi(state.ID.Instr.to_string().substr(0, 6))){
                case 100011:{
                    //lw instruction
                    newState.EX.alu_op = true;
                    newState.EX.is_I_type=true;
                    newState.EX.nop=false;
                    newState.EX.Imm=bitset<16>(state.ID.Instr.to_string().substr(16, 16));
                    newState.EX.rd_mem=true;
                    newState.EX.Read_data1=myRF.readRF(bitset<5>(state.ID.Instr.to_string().substr(6, 5)));  //$rs value
                    newState.EX.Read_data2=myRF.readRF(bitset<5>(state.ID.Instr.to_string().substr(11, 5))); //$rt value
                    newState.EX.Wrt_reg_addr=bitset<5>(state.ID.Instr.to_string().substr(11, 5));  //$rt addr
                    newState.EX.Rs=bitset<5>(state.ID.Instr.to_string().substr(6, 5));
                    newState.EX.Rt=bitset<5>(state.ID.Instr.to_string().substr(11, 5));
                    newState.EX.wrt_enable=true;
                    newState.EX.wrt_mem=false;

                    break;
                }
                case 101011:{
                    //sw instruction
                    newState.EX.alu_op = true;
                    newState.EX.is_I_type=true;
                    newState.EX.nop=false;
                    newState.EX.Imm=bitset<16>(state.ID.Instr.to_string().substr(16, 16));
                    newState.EX.rd_mem=false;
                    newState.EX.Read_data1=myRF.readRF(bitset<5>(state.ID.Instr.to_string().substr(6, 5)));  //$rs value
                    newState.EX.Read_data2=myRF.readRF(bitset<5>(state.ID.Instr.to_string().substr(11, 5))); //$rt value
                    newState.EX.Wrt_reg_addr=bitset<5>(state.ID.Instr.to_string().substr(11, 5));  //$rt addr
                    newState.EX.Rs=bitset<5>(state.ID.Instr.to_string().substr(6, 5));
                    newState.EX.Rt=bitset<5>(state.ID.Instr.to_string().substr(11, 5));
                    newState.EX.wrt_enable=false;
                    newState.EX.wrt_mem=true;     
             
                    break;
                }
                case 000000:{
                    if(state.ID.Instr.to_string().substr(29, 3)=="001"){
                        //addu
                        newState.EX.alu_op = true;
                        newState.EX.is_I_type=false;
                        newState.EX.nop=false;
                        newState.EX.Imm=0;
                        newState.EX.rd_mem=false;
                        newState.EX.Read_data1=myRF.readRF(bitset<5>(state.ID.Instr.to_string().substr(6, 5)));
                        newState.EX.Read_data2=myRF.readRF(bitset<5>(state.ID.Instr.to_string().substr(11, 5)));
                        newState.EX.Wrt_reg_addr=bitset<5>(state.ID.Instr.to_string().substr(16, 5));
                        newState.EX.Rs=bitset<5>(state.ID.Instr.to_string().substr(6, 5));
                        newState.EX.Rt=bitset<5>(state.ID.Instr.to_string().substr(11, 5));
                        newState.EX.wrt_enable=true;
                        newState.EX.wrt_mem=false;
                        
                    }else if(state.ID.Instr.to_string().substr(29, 3)=="011"){
                        //subu
                        newState.EX.alu_op = false;
                        newState.EX.is_I_type=false;
                        newState.EX.nop=false;
                        newState.EX.Imm=0;
                        newState.EX.rd_mem=false;
                        newState.EX.Read_data1=myRF.readRF(bitset<5>(state.ID.Instr.to_string().substr(6, 5)));
                        newState.EX.Read_data2=myRF.readRF(bitset<5>(state.ID.Instr.to_string().substr(11, 5)));
                        newState.EX.Wrt_reg_addr=bitset<5>(state.ID.Instr.to_string().substr(16, 5));
                        newState.EX.Rs=bitset<5>(state.ID.Instr.to_string().substr(6, 5));
                        newState.EX.Rt=bitset<5>(state.ID.Instr.to_string().substr(11, 5));
                        newState.EX.wrt_enable=true;
                        newState.EX.wrt_mem=false;

                    }
                    
                    break;
                }
                case 100:{
                    bitset<32> branch_reg1=myRF.readRF(bitset<5>(state.ID.Instr.to_string().substr(6, 5)));
                    bitset<32> branch_reg2=myRF.readRF(bitset<5>(state.ID.Instr.to_string().substr(11, 5)));
                    bitset<32> branch_address=get_Branch_Addr(bitset<16>(state.ID.Instr.to_string().substr(16, 16)));
                    //cout<<"2 reg"<<endl<<branch_reg1<<endl<<branch_reg2<<endl;
                    if(branch_reg1==branch_reg2){
                        //do nothing
                        newState.EX.nop=false;
                    }else{
                        //nop
                        newState.EX.nop=true;
                        newState.ID.nop=true;
                        //PC update
                        do_Branch=true;
                        newState.IF.PC=bitset<32>(state.IF.PC.to_ulong() + 4 + branch_address.to_ulong());

                    }
                    break;
                }
            }

        }else{
            newState.EX.nop=true;
        }



        /* --------------------- IF stage --------------------- */
        if(state.IF.nop==false){
            bitset<32> _current_instruction = myInsMem.readInstr(state.IF.PC);

            //cout<<state.IF.PC.to_ulong()<<endl;

            if(_current_instruction.to_ulong()== 0xffffffff){
                newState.IF.nop=true;
            }else{
                //for next ID
                newState.ID.Instr=_current_instruction;
                newState.ID.nop=state.IF.nop;
                //PC+4

                if(do_Branch==false){
                    newState.IF.PC=bitset<32>(state.IF.PC.to_ulong() + 4);
                }else{
                    do_Branch==false;
                }

            }
        }else{
            newState.IF.nop=true;
            newState.ID.nop=true;
        }
        
        /* --------------------- Stall unit--------------------- */


        if (state.IF.nop && state.ID.nop && state.EX.nop && state.MEM.nop && state.WB.nop)
            break;
        
        printState(newState, cycle); //print states after executing cycle 0, cycle 1, cycle 2 ... 

        cycle += 1;
        state = newState; /*The end of the cycle and updates the current state with the values calculated in this cycle */

        //cout<<"next state nop bit:"<<state.IF.nop<<state.ID.nop<<state.EX.nop<<state.MEM.nop<<state.WB.nop<<endl;

    }

    myRF.outputRF(); // dump RF;	
    myDataMem.outputDataMem(); // dump data mem 

    return 0;
}