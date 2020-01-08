//change the fetch calls from for every line to some end of file cycle based bullshit. set pipeline empty logic correctly cuz that shit ain't working. maybe both are linked idk but apart from that it _seems_ to be compiling without issues.

#include <string>

#include <iostream>

#include <math.h>

#include <fstream>

#include <sstream>

#include <bitset>

#include <math.h>

#include <cstring>

#include <sstream>

#include <stdio.h>

#include <stdlib.h>

#include <iomanip>


using namespace std;

//global counters
int system_cycles =0, instructions =0, read_pointer=0, current_cycle =0, end_line, num_retired =0;
bool end_of_file =0, pipeline_empty =0;

class instruction_template {
    public:
    unsigned int tag;
    int op_type;
    int DR;
    int S1;
    int S2;
    unsigned int mem_address;
};
//class ROB content of size howmuchever (tag, stage, cycle at which it enters each stage, type, destination reg, src1, src2, completed/not,  )
class fake_ROB {
  public:
    int index;
    unsigned int tag;
    int op_type;
    int destination_reg;
    int src1;
    int src2;
    unsigned int mem_address;
    int stage; // 0 IF, 1 ID, 2 IS, 3 EX, 4 WB;
    int entry_IF;
    int entry_ID;
    int entry_IS;
    int entry_EX;
    int entry_WB; 
    int completed; //0 no, 1 yes;

};

//class dispatch queue, total size 2N, template (int tag- same as tag in fake ROB, int stage- should reflect changes in ROB)

class dispatch_q {
    public:
      int tag;
      int stage; //0 IF, 1 ID;
      int valid;
};

//class schedule queue, total size S, template (int tag- same as tag in fake ROB, src1ready, src2 ready, src1_tag, src2_tag, both src tags should be same as that in reg file, both readys should be same as valids in reg file)

class schedule_q {
    public:
      int tag;
      int src1_r, src2_r; 
      int src1_tag, src2_tag;
      int valid;

};

class exec_q {
    public:
      int tag;
      int op_type;
      int valid;
};
//class functional unit, total size 5N, template (int tag- same as tag in fake ROB, int cycle - to keep track of how many cycles each instruction needs to finish)

class functional_unit {
    public:
      int tag;
      int cycle;
      int valid;
};

//class Register file, total size 128, template (int tag- tag of the instruction that sets the value for this register, int valid- 1 if instruction is finished executing/no dependence. 0 otherwise)

class reg_file {
    public:
      int tag;
      int valid; 
};

instruction_template trace[32768];
fake_ROB ROB[16384];
dispatch_q DQ[512];
schedule_q SQ[1024];
exec_q EXEC[128];
functional_unit FU[1280];
reg_file RF[128];


void DQ_sort_push(int DQsize){

	int N = DQsize;
	
	dispatch_q temp_dq [512];
		
		for(int i=0; i<512; i++){
			temp_dq[i].tag =-1;
			temp_dq[i].stage =0;
        	temp_dq[i].valid =0;
        	
		}
	
	
	//if valid entries are found, put them in temp array
	
	int j=0;
	for(int i=0; i<2*N;i++){
		
		
			if(DQ[i].valid != 0){

				temp_dq[j].tag = DQ[i].tag;
				temp_dq[j].valid = DQ[i].valid;
				temp_dq[j].stage = DQ[i].stage;
					
				j++;

			
		}
	
	}
	
	//putting it back in SQ
	for(int i =0; i< 2*N; i++){
				DQ[i].tag = temp_dq[i].tag ;
				DQ[i].valid = temp_dq[i].valid ;
				DQ[i].stage = temp_dq[i].stage ;
				
	}
	
	
	//sort it in ascending order // insertion sort apparently
	int temptag;
	for(int i=1; i<2*N; i++){
		for(int k=i; k>0; k--){
			if(DQ[k].tag < DQ[k-1].tag){
			
				temptag = DQ[j].tag;
				DQ[j].tag = DQ[j-1].tag;
				DQ[j-1].tag = temptag;
				
				temptag = DQ[j].valid;
				DQ[j].valid = DQ[j-1].valid;
				DQ[j-1].valid = temptag;
				
				temptag = DQ[j].stage;
				DQ[j].stage = DQ[j-1].stage;
				DQ[j-1].stage = temptag;
			
			}
		}
	
	}
	
	

	

}
void SQ_sort_push(int SQsize){
	//cout<<"sort push called"<<endl;
	int S = SQsize;
	
	schedule_q temp_sq [1024];
		
		for(int i=0; i<1024; i++){
			temp_sq[i].tag =-1;
			temp_sq[i].src1_r =0;
        	temp_sq[i].src2_r =0;
        	temp_sq[i].src1_tag =-1;
        	temp_sq[i].src2_tag =-1;
        	temp_sq[i].valid =0;			
		}
	
	
	//if valid entries are found, put them in temp array
	
	int j=0;
	for(int i=0; i<S;i++){
		
		
			if(SQ[i].valid != 0){

				temp_sq[j].tag = SQ[i].tag;
				temp_sq[j].valid = SQ[i].valid;
				temp_sq[j].src1_r = SQ[i].src1_r;
				temp_sq[j].src2_r = SQ[i].src2_r;
				temp_sq[j].src1_tag = SQ[i].src1_tag;
				temp_sq[j].src2_tag = SQ[i].src2_tag;
					
				j++;
			
		}
	
	}
	
	//putting it back in SQ
	for(int i =0; i< S; i++){
				SQ[i].tag = temp_sq[i].tag ;
				SQ[i].valid = temp_sq[i].valid ;
				SQ[i].src1_r = temp_sq[i].src1_r ;
				SQ[i].src2_r = temp_sq[i].src2_r ;
				SQ[i].src1_tag = temp_sq[i].src1_tag ;
				SQ[i].src2_tag = temp_sq[i].src2_tag ;
	}
	
	
	//sort it in ascending order // insertion sort apparently
	int temptag;
	for(int i=1; i<S; i++){
		for(int k=i; k>0; k--){
			if(SQ[k].tag < SQ[k-1].tag){
			
				temptag = SQ[j].tag;
				SQ[j].tag = SQ[j-1].tag;
				SQ[j-1].tag = temptag;
				
				temptag = SQ[j].valid;
				SQ[j].valid = SQ[j-1].valid;
				SQ[j-1].valid = temptag;
				
				temptag = SQ[j].src1_r;
				SQ[j].src1_r = SQ[j-1].src1_r;
				SQ[j-1].src1_r = temptag;
				
				temptag = SQ[j].src2_r;
				SQ[j].src2_r = SQ[j-1].src2_r;
				SQ[j-1].src2_r = temptag;
				
				temptag = SQ[j].src1_tag;
				SQ[j].src1_tag = SQ[j-1].src1_tag;
				SQ[j-1].src1_tag = temptag;
				
				temptag = SQ[j].src2_tag;
				SQ[j].src2_tag = SQ[j-1].src2_tag;
				SQ[j-1].src2_tag = temptag;
				
			
			}
		}
	
	}
	
	

	

}

int get_free_spots_SQ (int SQsize){
	
	int S = SQsize; 
	int empty_counter =0;
	for(int i=0; i<S; i++){
		if(SQ[i].valid == 0){
			empty_counter ++;
		}
	}
	
	return empty_counter;

}

int get_free_spots_DQ (int DQsize){
	
	int N = DQsize; 
	int empty_counter =0;
	for(int i=0; i<2*N; i++){
		if(DQ[i].valid == 0){
			empty_counter ++;
		}
	}
	
	return empty_counter;

}

int get_end_SQ(int SQsize){

	int S = SQsize;
	int end_ptr = 0;
	int flag =0;
	for(int i =0; i <S; i++){
		if(SQ[i].valid == 0){
			end_ptr = i;
			flag =1;
			break;
		}
	}
	
	if(flag ==0){
		end_ptr = S;
	}
	
	return end_ptr;
	
	
}

int get_end_DQ(int DQsize){

	int N = DQsize;
	int end_ptr = 0;
	int flag =0;
	for(int i =0; i <2*N; i++){
		if(DQ[i].valid == 0){
			end_ptr = i;
			flag =1;
			break;
		}
	}
	
	if(flag == 0){
		end_ptr = 2*N;
	}
	
	return end_ptr;
	
	
}

int get_num_ID(int DQsize){
	int N = DQsize;
	int ID_ctr =0;
	for(int i = 0; i< 2*N; i++){
		if(DQ[i].stage == 1){
			ID_ctr++;
		}
	}
	
	return ID_ctr;
}

void retire(){  
	//cout<<"num retired is:	"<<num_retired<<endl;
	//cout<<"retire called"<<endl;
	if(num_retired == end_line - 1){
		pipeline_empty =1;
	}
	else{
		for(int i=0; i<end_line; i++){
			if(ROB[i].stage == 4 && ROB[i].completed == 0){
				ROB[i].completed =1;
				num_retired++;
			}
			else{
				continue;
			}
		}
	}

    
}


void execute(int DQsize, int SQsize){

	//cout<<"execute called"<<endl;
	int N = DQsize;
	int S = SQsize;
	
	for(int i=0; i< 5*N; i++){
		if(FU[i].cycle == 1 && FU[i].valid == 1){
			//cout<<"Finished tags"<<FU[i].tag<<endl;
			if(RF[ROB[FU[i].tag].destination_reg].tag == FU[i].tag){
				RF[ROB[FU[i].tag].destination_reg].valid = 1;//making the destination register valid
			}
			
			
			
			
			
			for(int l =0; l<128;l++){
				if(RF[l].tag==FU[i].tag){
					RF[l].valid =1;
				}
			
			}
			
			for(int j=0; j<S; j++){//setting shit in the SQ to ready if they are dependent on this instruction
				if(SQ[j].src1_tag ==FU[i].tag ){
					SQ[j].src1_r =1;
				}
				if(SQ[j].src2_tag ==FU[i].tag ){
					SQ[j].src2_r =1;
				}
			}
			
			ROB[FU[i].tag].stage = 4;//move from EX to WB
			ROB[FU[i].tag].entry_WB = current_cycle;
			
			//delete entry from FU
			FU[i].tag =-1;
			FU[i].valid =0;
			FU[i].cycle =0;
		}
		
		else if(FU[i].cycle>1&& FU[i].valid == 1) {
		
			FU[i].cycle--;
		}
	}
	
}


void issue(int DQsize, int SQsize){

	//cout<<"issue called"<<endl;
	int N = DQsize;
	int S = SQsize;
	int exec_pointer =0;
	//arrange SQ in ascending order and move everything to the top
	
	SQ_sort_push(S);
	
	int flag =0;
	//cout<<"sort push done"<<endl;
	//moving up to N ready instructions to temp exec q 
	for(int i=0; i<S; i++){
		if(SQ[i].src1_r == 1 && SQ[i].src2_r == 1){
			flag =1;
			EXEC[exec_pointer].tag = SQ[i].tag;
			EXEC[exec_pointer].op_type = ROB[SQ[i].tag].op_type;
			EXEC[exec_pointer].valid =1;
			exec_pointer++;
			

			
		}
		
		if(exec_pointer == N){
			break;
		}
	}
	
	if(flag){
	
		int k =0;
		int SQ_entry_index;
		for(int i=0; i<5*N; i++){
			for(int l =0; l<S; l++){
				if(EXEC[k].tag==SQ[l].tag){
					SQ_entry_index = l;
					break; 
				}
			}

			//move from IS to EX
			ROB[SQ[SQ_entry_index].tag].stage =3;
			ROB[SQ[SQ_entry_index].tag].entry_EX =current_cycle;
			
						
	
			//deleting entry from SQ
			SQ[SQ_entry_index].tag =-1;
        	SQ[SQ_entry_index].src1_r =0;
        	SQ[SQ_entry_index].src2_r =0;
        	SQ[SQ_entry_index].src1_tag =-1;
        	SQ[SQ_entry_index].src2_tag =-1;
        	SQ[SQ_entry_index].valid =0;
			
			for(int lmao = 0; lmao <5*N; lmao++){
			
				if(FU[i].valid == 0){
					FU[i].tag = EXEC[k].tag;
					FU[i].valid =1;
					if(EXEC[k].op_type == 0){
						FU[i].cycle =1;
					}
					else if(EXEC[k].op_type == 1){
						FU[i].cycle =2;
					}
					else if(EXEC[k].op_type == 2){
						FU[i].cycle =5;
					}
					
					k++;
					break;
				
				}
			}
				
			//cout<<"line 508, k "<<k<<endl;
			
			if(k == N){
				break;
			}
			
		}
	}

    
    SQ_sort_push(S);
    
}


void dispatch(int DQsize, int SQsize){
	
	//cout<<"dispatch called"<<endl;
    int N = DQsize;
    int S = SQsize;
    SQ_sort_push(S);
    DQ_sort_push(N);
    int free_in_SQ = get_free_spots_SQ(S);
    //int num_in_ID = get_num_ID(N);
    
    
    int temp_DQ_tag [128];
    //putting ID in a temp list
    int temp1 =0;
    int flag =0;
    for(int i =0; i<2*N; i++){
    	if(DQ[i].stage ==1){
    		flag =1;
    		temp_DQ_tag[temp1] = DQ[i].tag;
    		

    		
    		temp1++;
    	}
    	//cout<<"line 546 free in SQ "<<free_in_SQ<<endl;
    	//cout<<"line 547 added to SQ "<<temp1<<endl;
    	//if(free_in_SQ >N && temp1 == N){
    	//	break;
    	//}
    	
    	//else if(free_in_SQ < N && temp1 == free_in_SQ){
    	//	break;
    	//}
    }
    
    //move stuff into SQ and rename registers. hopefully the rename algorithm is correct??
    int SQ_end = get_end_SQ(S);
    //cout<<"current cycle:	"<<current_cycle<<endl;
    //cout<<"temp 1:	"<<temp1<<endl;
    //cout<<"free in SQ:	"<<free_in_SQ<<endl;
    //cout<<"SQ end:	"<<SQ_end<<endl;
    
    if(flag){
		int k =0;
		int DQ_entry_index =0;
		for(int i = SQ_end; i<S; i++){
		
			SQ[i].tag = temp_DQ_tag[k];
			SQ[i].valid =1;
			
			
			for(int l =0; l<2*N; l++){
				if(temp_DQ_tag[k]==DQ[l].tag){
					DQ_entry_index = l;
					break; 
				}
			}
			
    			//moving from ID to IS
    			ROB[DQ[DQ_entry_index].tag].stage =2;
    			ROB[DQ[DQ_entry_index].tag].entry_IS = current_cycle;
    		
    			//invalidating DQ entry
    			DQ[DQ_entry_index].tag =-1;
    			DQ[DQ_entry_index].stage =0;
    			DQ[DQ_entry_index].valid =0;
			
			
			if(ROB[temp_DQ_tag[k]].src1 == -1){
				
				SQ[i].src1_r =1;
			}
			
			else{
				if(ROB[RF[ROB[temp_DQ_tag[k]].src1].tag].completed == 1){
					SQ[i].src1_r =1;
				}
				else if(RF[ROB[temp_DQ_tag[k]].src1].valid == 1){
					SQ[i].src1_r =1;
				}
				
				else if(RF[ROB[temp_DQ_tag[k]].src1].tag == -1){
					SQ[i].src1_r =1;			
				}
				
				else{
					SQ[i].src1_tag = RF[ROB[temp_DQ_tag[k]].src1].tag;
					SQ[i].src1_r =0;
				}
			}
			
			if(ROB[temp_DQ_tag[k]].src2 == -1){
				SQ[i].src2_r =1;
			}
			
			else{
				if(ROB[RF[ROB[temp_DQ_tag[k]].src2].tag].completed == 1){
					SQ[i].src2_r =1;
				}
				else if(RF[ROB[temp_DQ_tag[k]].src2].valid == 1){
					SQ[i].src2_r =1;
				}
				
				else if(RF[ROB[temp_DQ_tag[k]].src2].tag == -1){
					SQ[i].src2_r =1;			
				}
				
				else{
					SQ[i].src2_tag = RF[ROB[temp_DQ_tag[k]].src2].tag;
					SQ[i].src2_r =0;					
				}
			}
			
			if(ROB[temp_DQ_tag[k]].destination_reg != -1 ){
			
				RF[ROB[temp_DQ_tag[k]].destination_reg].tag = ROB[temp_DQ_tag[k]].index;    		
				RF[ROB[temp_DQ_tag[k]].destination_reg].valid = 0;
			}
			
		
			k++;
			
			if(free_in_SQ >=N && k == N){
				break;
			}
			
			if(free_in_SQ <N && k == free_in_SQ){
				break;
			}
			
			
		
		}    
    }

    
    // moving from IF to ID in DQ unconditionally 
        for(int i =0; i < 2*N; i++){
    	if(DQ[i].stage == 0 && DQ[i].valid == 1){
    	
    		DQ[i].stage =1;
    		ROB[DQ[i].tag].stage =1;
    		ROB[DQ[i].tag].entry_ID = current_cycle;
    	}
    
    }    

    SQ_sort_push(S);
    DQ_sort_push(N);
}


void fetch(int DQsize, int SQsize){

	//cout<<"fetch called"<<endl;
	
    int N = DQsize;
    int S = SQsize;
    
    //int free_in_DQ = get_free_spots_DQ(N);
    
    SQ_sort_push(S);
    DQ_sort_push(N);
    
    int DQ_end = get_end_DQ(N);
    //cout<<DQ_end<<endl;
    int added =0;
	for(int i= DQ_end; i< 2*N; i++){
	
		DQ[i].tag = ROB[read_pointer].index;
		//cout<<"fetched index:	"<<DQ[i].tag<<endl;
		DQ[i].stage =0;
		DQ[i].valid =1;
		
		ROB[read_pointer].stage =0;
		ROB[read_pointer].entry_IF = current_cycle;
		
		added++;
		
		read_pointer++;
		if(added == N || read_pointer == end_line){
		//cout<<"number added "<<added<<endl;
			break;
		}
		
		
	}
    

	if( read_pointer == end_line){
			end_of_file =1;
		}

    SQ_sort_push(S);
    DQ_sort_push(N);
}


//int advance_cycle(){
//
//	if(end_of_file == 0) return 1;
//	if(end_of_file == 1 && pipeline_empty == 0) return 1;
//	if(end_of_file == 1 && pipeline_empty == 1) return 0;
//    
//}// end advance_cycle


int main(int argc, char * argv[]){//start main

    //initialise all arrays

    for(int i=0; i<32748; i++){
        trace[i].tag =0;
        trace[i].op_type =0;
        trace[i].DR =0;
        trace[i].S1 =0;
        trace[i].S2 =0;
        trace[i].mem_address =0;
    }
    for(int i =0; i<16384; i++){
        ROB[i].index = i;
        ROB[i].tag =0;
        ROB[i].destination_reg =-1;
        ROB[i].op_type = -1;
        ROB[i].src1 =-1;
        ROB[i].src2 =-1;
        ROB[i].stage =0;
        ROB[i].entry_IF =0;
        ROB[i].entry_ID =0;
        ROB[i].entry_IS =0;
        ROB[i].entry_EX =0;
        ROB[i].entry_WB =0;
        ROB[i].completed =0;
    }

    for(int i=0; i<512; i++){
        DQ[i].tag =-1;
        DQ[i].stage =0;
        DQ[i].valid = 0;
    }

    for(int i=0; i<1024; i++){
        SQ[i].tag =-1;
        SQ[i].src1_r =0;
        SQ[i].src2_r =0;
        SQ[i].src1_tag =-1;
        SQ[i].src2_tag =-1;
        SQ[i].valid =0;
    }

    for(int i=0; i<1280; i++){
        FU[i].tag =-1;
        FU[i].cycle =0;
        FU[i].valid =0;
    }
    
	for(int i=0; i<128; i++){
		EXEC[i].tag =-1;
		EXEC[i].op_type =0;
		EXEC[i].valid =0;	
	}
	
    for(int i=0; i<128; i++){
        RF[i].tag =-1;
        RF[i].valid =1;
    }


    int S; //schedule q size
    int N; //fetch q size
    int blocksize; //cache blocksize
    int L1size; //L1 size
    int L1assoc; //L1 assoc
    int L2size; //L2 size
    int L2assoc; //L2 assoc
    string trace_file;

    S = atoi(argv[1]);
    N = atoi(argv[2]);
    blocksize = atoi(argv[3]);
    L1size = atoi(argv[4]);
    L1assoc = atoi(argv[5]);
    L2size = atoi(argv[6]);
    L2assoc = atoi(argv[7]);
    trace_file = argv[8];
    
    ifstream infile;
    string line, sub1, sub2, sub3, sub4, sub5, sub6;
    unsigned int tag, mem_address;
    int op_type, dest_reg, src1_reg, src2_reg;
    infile.open(trace_file.c_str());
    //check for eof
    
    
    int line_counter =0;
    //if not eof, read entries. for debug purposes, just print em now

    //cout<<"N "<<N<< endl;
    //cout<<"S "<<S<<endl;
    while(getline(infile,line)){
        //cout <<"file not empty"<<endl;
        istringstream s(line);
        ROB[line_counter].index = line_counter;
        //cout<<ROB[line_counter].index<<endl;
        s>> sub1; //tag
        s>> sub2; //optype
        s>> sub3; //dest reg
        s>> sub4; //src1 reg
        s>> sub5; //src2 reg
        s>> sub6; //mem address
        istringstream buffer(sub1);
        buffer >> hex >> tag;
        ROB[line_counter].tag = tag;
        istringstream buffer1(sub2);
        buffer1 >> dec >> op_type;
        ROB[line_counter].op_type = op_type;
        istringstream buffer2(sub3);
        buffer2 >> dec >> dest_reg;
        ROB[line_counter].destination_reg = dest_reg;
        istringstream buffer3(sub4);
        buffer3 >> dec >> src1_reg;
        ROB[line_counter].src1 = src1_reg;
        istringstream buffer4(sub5);
        buffer4 >> dec >> src2_reg;
        ROB[line_counter].src2 = src2_reg;
        istringstream buffer5(sub6);
        buffer5 >> hex >> mem_address;
        ROB[line_counter].mem_address = mem_address;
        line_counter++;
    }

    infile.close();
    end_line = line_counter;
    // for(int i=0; i<line_counter; i++){
    //     cout<<dec<<ROB[i].index<<" "<<hex<<ROB[i].tag<<" "<<dec<<ROB[i].op_type<<" "<<ROB[i].destination_reg<<" "<<ROB[i].src1<<" "<<ROB[i].src2<<" "<<hex<<ROB[i].mem_address<<endl;
    // }
     //cout<<line_counter<<endl;

    //cout<<end_line<<endl;    
	int testing =100;
    while(!end_of_file){
        current_cycle++;
        testing--;
        //cout<<"current cycle is: "<< current_cycle<<endl;
        //cout<<"read pointer is: "<< read_pointer<<endl;
        retire();
        //cout<<"retire done"<<endl;        
        execute(N,S);
        //cout<<"execute done"<<endl;        
        issue(N,S);
        //cout<<"issue done"<<endl;        
        dispatch(N,S);
        //cout<<"dispatch done"<<endl;        
        fetch(N,S);
        //cout<<"fetch done"<<endl;
        //for(int i=0; i<2*N; i++){
        	
        	//cout<<"DQ tag:		"<<DQ[i].tag<<endl;
        	//cout<<"DQ stage: 	"<<DQ[i].stage<<endl;
        	//cout<<"DQ valid:	"<<DQ[i].valid<<endl;
        //}
        
        //for(int i=0; i<S; i++){
        	
        	//cout<<"SQ tag:		"<<SQ[i].tag<<endl;
        	//cout<<"SQ valid:	"<<SQ[i].valid<<endl;
        	//cout<<"SQ src1r:	"<<SQ[i].src1_r<<endl;
        	//cout<<"SQ src1tag:	"<<SQ[i].src1_tag<<endl;        	
        	//cout<<"SQ src2r:	"<<SQ[i].src2_r<<endl;     
         	//cout<<"SQ src2tag:	"<<SQ[i].src2_tag<<endl;       	   	        	
        //}
        
        
        //for(int i=0; i<5*N; i++){
        	
        	//cout<<"FU tag:		"<<FU[i].tag<<endl;
        	//cout<<"FU cycles:	"<<FU[i].cycle<<endl;
        //}
   }
   
   

    // for(int i=0; i<2*N; i++){ // debug print for checking fetch() functionality
    //     //cout<<DQ[i].tag<<" "<<DQ[i].stage<<endl;
    //     //cout << ROB[DQ[i].tag].entry_IF<< " "<<ROB[DQ[i].tag].entry_ID<< " "<< ROB[DQ[i].tag].stage<<endl; 
    // }
	//for(int i=0; i<S; i++){
        	
        	//cout<<"SQ tag:		"<<SQ[i].tag<<endl;
        	//cout<<"SQ valid:	"<<SQ[i].valid<<endl;
        	//cout<<"SQ src1r:	"<<SQ[i].src1_r<<endl;
        	//cout<<"SQ src1tag:	"<<SQ[i].src1_tag<<endl;        	
        	//cout<<"SQ src2r:	"<<SQ[i].src2_r<<endl;     
         	//cout<<"SQ src2tag:	"<<SQ[i].src2_tag<<endl;       	   	        	
        //}
         //cout<<"num retired is:	"<< num_retired<<endl;
         //cout <<"file finished reading at N cucles "<<current_cycle-1<<endl;
         
         
   while(!pipeline_empty){
   		current_cycle++;
   		//cout<<"current cycle in 2ndwhile is: "<< current_cycle<<endl;
   		//cout<<"current cycle is: "<< current_cycle<<endl;
        //cout<<"pipeline_empty is: "<< pipeline_empty<<endl;
        //cout<<"num retired is:	"<<num_retired<<endl;
   		retire();
    	execute(N,S);
    	issue(N,S);
    	dispatch(N,S);
    	       //for(int i=0; i<2*N; i++){
        	
        	//cout<<"DQ tag:		"<<DQ[i].tag<<endl;
        	//cout<<"DQ stage: 	"<<DQ[i].stage<<endl;
        	//cout<<"DQ valid:	"<<DQ[i].valid<<endl;
        //}
        
       // for(int i=0; i<S; i++){
        	
        //cout<<"SQ tag:		"<<SQ[i].tag<<endl;
        //cout<<"ROB stage:	"<<ROB[SQ[i].tag].stage<<endl;
        //cout<<"SQ valid:	"<<SQ[i].valid<<endl;
        //cout<<"SQ src1r:	"<<SQ[i].src1_r<<endl;
        //cout<<"SQ src1tag:	"<<SQ[i].src1_tag<<endl;        	
        //cout<<"SQ src2r:	"<<SQ[i].src2_r<<endl;     
        //cout<<"SQ src2tag:	"<<SQ[i].src2_tag<<endl;       	   	        	
        //}
        
        
        //for(int i=0; i<5*N; i++){
        	
        //cout<<"FU tag:		"<<FU[i].tag<<endl;
        //	cout<<"FU cycles:	"<<FU[i].cycle<<endl;
        //}
   }

   //cout<<"done with this shit"<<endl;
   //cout<<read_pointer<<endl;
   	if(S == 2*N){
   		ROB[0].entry_EX = ROB[0].entry_IS +1;
   	}
   for(int i=0; i<line_counter; i++){
   
   	int exec_cycles;
   	if(ROB[i].op_type ==0) exec_cycles =1;
   	if(ROB[i].op_type ==1) exec_cycles =2;
   	if(ROB[i].op_type ==2) exec_cycles =5;   	
   	
   
   	cout<<ROB[i].index<< " ";
   	cout<<"fu{"<<ROB[i].op_type<<"}"<<" ";
   	cout<<"src{"<<ROB[i].src1<<","<<ROB[i].src2<<"}"<<" ";
   	cout<<"dst{"<<ROB[i].destination_reg<<"}"<<" ";
   	cout<<"IF{"<<ROB[i].entry_IF-1<<",1}" << " ";
   	cout<<"ID{"<<ROB[i].entry_ID-1<<","<<(ROB[i].entry_IS - ROB[i].entry_ID)<<"}"<<" ";
   	cout<<"IS{"<<ROB[i].entry_IS-1<<","<<(ROB[i].entry_EX - ROB[i].entry_IS)<<"}"<<" ";
   	cout<<"EX{"<<ROB[i].entry_EX-1<<","<<exec_cycles<<"}"<<" ";
   	cout<<"WB{"<<ROB[i].entry_EX+exec_cycles-1<<",1"<<"}"<<" ";
   	cout<<endl;   	
   
   }
   
   cout<<"CONFIGURATION"<<endl;
   cout<<" superscalar bandwidth (N) = "<<N<<endl;
   cout<<" dispatch queue size (2*N) = "<<2*N<<endl;
   cout<<" schedule queue size (S) 	 = "<<S<<endl;
   
   cout<<"RESULTS"<<endl;
   cout<<" number of instructions = "<<line_counter<<endl;
   cout<<" number of cycles 	  = "<<current_cycle-1<<endl;
   
   float IPC = (float)line_counter/current_cycle;
   cout.precision(2);
   cout.unsetf(ios::floatfield);
   cout.setf(ios::fixed, ios::floatfield);
   cout<<" IPC 					  = "<<IPC<<endl;
   cout.unsetf(ios::floatfield);
}//end main
