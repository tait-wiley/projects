

// Tait Wiley - 10064664

#include "memsim.h"
#include <cassert>
#include <iostream>
#include <list>
#include <iterator>
#include <set>
#include <map>
#include <cmath>



using namespace std;

struct Partition {
  // the process id if being used by that process, -1 if currently free
  int tag;
  // amount of memory partition is taking up
  int64_t size;
  // address of earlies memory spot of partition 
  // = (address of previous partition + size of previous partition)
  int64_t address;

  // constructor
  Partition(int tag_, int64_t size_, int64_t address_) :   tag(tag_), size(size_), address(address_)
  {
  }
};

// PR, pointer to a partition in a list of partitions
typedef list<Partition>::iterator PartitionRef;

// for ordering the partition pointers by size in SET
struct scmp { 
  bool operator()(const PartitionRef & c1, const PartitionRef & c2) const { 
    if (c1->size == c2->size)  
      return c1->address < c2->address; 
    else 
      return c1->size > c2->size; 
  } 
};

// helper function for printing a list of partitions (from tutorial)
void printList(list<Partition> &partitionList){
    cout << "list contents are:" << endl;
    for (list<Partition>::iterator it = partitionList.begin(); it != partitionList.end(); it++)
    {
        cout << it->tag << ", " << it->size << ", " << it->address <<endl;
    }
}

// helper function for printing a set of partition refs (from tutorial)
void printSet(set<PartitionRef,scmp> &partitionRefSet){
    cout << "set contents are:" << endl;
     for (set<PartitionRef,scmp>::iterator it = partitionRefSet.begin(); it != partitionRefSet.end(); ++it)
    {
        cout << (*it)->tag << ", " << (*it)->size << ", " << (*it)->address << endl;
    }
}

// helper function for printing a set of partition refs (from tutorial)
void printMap(map<long, vector<PartitionRef>> &tagged_map){
    cout << "map sizes are:" << endl;
    for(size_t i = 0; i < tagged_map.size(); ++i){
      cout << "size of tagged map " << i << " is " << tagged_map[i].size() << endl;
    }
}

struct Simulator {
  // to store the page size given
  int64_t pageSize;

  // tracking the total number of pages requested
  int64_t pages_requested = 0;

  // all partitions, in a linked list 
  list<Partition> all_blocks; 

  // sorted partitions by size/address 
  set<PartitionRef,scmp> free_blocks; 

  // quick access to all tagged partitions 
  map<long, vector<PartitionRef>> tagged_blocks;

  // constructor
  Simulator(int64_t page_size) : pageSize(page_size)
  {    
  }

  // setting up intial 0 size partition
  void setup(){
    Partition setupPARTITION = Partition(-1, 0, 0);  
    all_blocks.insert(all_blocks.end(), setupPARTITION);
    
    PartitionRef setupPR = all_blocks.begin();
    free_blocks.insert(setupPR);

  }


  void allocate(int tag, int size)
  {
    // request with size and tag has come in

    // CASE: NEW PAGES ARE REQUIRED TO FULFILL IT 
    if(free_blocks.size() == 0){

      int64_t pages_required = 0;

      // CASE: LAST PARTITION IS FREE (-1)
      if(prev(all_blocks.end()) -> tag < 0){    
         //cout << "CASE: LAST PARTITION IS FREE on tag " << tag << endl;  
        // determine number of pages required   
        int total_Req_mem = size - (prev(all_blocks.end()) -> size);

        if(total_Req_mem <= pageSize){
          pages_required = 1;
          pages_requested += 1;
        }
        
        else{          
          while(true){
            total_Req_mem -= pageSize;
            pages_required += 1;
            pages_requested += 1;
            if(total_Req_mem <= 0){
              break;
            }
          }
        }

        // create a new free partition of size = pageSize*pages_required + the previous partition size
        Partition newPartition0 = Partition(-1, (pageSize*pages_required) + (prev(all_blocks.end()) -> size), (prev(all_blocks.end()) -> address));

        // add partition to end of all blocks
        all_blocks.insert(all_blocks.end(), newPartition0);

        // create REF to that partition
        PartitionRef newPartitionRef0 = prev(all_blocks.end());

        // add that partition to set
        free_blocks.insert(newPartitionRef0);

        // delete old partition from set and list
        free_blocks.erase(prev(newPartitionRef0));
        all_blocks.erase(prev(newPartitionRef0));
        
        // insert the process regularly 
        // INSERTION ALGORITHM ################

        // get largest free partition
        set<PartitionRef,scmp>::iterator PR_set = free_blocks.begin();
        // if the same size as the process
        if((*PR_set) -> size == size){          
          (*PR_set) -> tag = tag;
          tagged_blocks[tag].push_back((*PR_set));
          free_blocks.erase(PR_set);
        }

        // if larger need to split
        else{          

          PartitionRef old_Ref0 = (*PR_set);
          old_Ref0 -> size = (*PR_set) -> size - size;
          old_Ref0 -> address = (*PR_set) -> address + size;

          Partition newPARTITION = Partition(tag, size, ((*PR_set) -> address) - size);
          all_blocks.insert((*PR_set), newPARTITION);
          PartitionRef newPR = prev(*PR_set);
          tagged_blocks[tag].push_back(newPR);

          free_blocks.erase(PR_set);
          free_blocks.insert(old_Ref0);
        }

        // ###################################

      }// END CASE: LAST PARTITION IS FREE (-1)


      // CASE: LAST PARTITION IS NOT FREE (2)
      else{
        // determine number of pages required   
        int total_Req_mem = size;

        if(total_Req_mem <= pageSize){
          pages_required = 1;
          pages_requested += 1;
        }
        
        else{          
          while(true){
            total_Req_mem -= pageSize;
            pages_required += 1;
            pages_requested += 1;
            if(total_Req_mem <= 0){
              break;
            }
          }
        }

        // create a new free partition of size = pageSize*pages_required
        Partition newPartition1 = Partition(-1, (pageSize*pages_required), (prev(all_blocks.end()) -> address) +  (prev(all_blocks.end()) -> size));

        // add partition to end of all blocks
        all_blocks.insert(all_blocks.end(), newPartition1);

        // create REF to that partition
        PartitionRef newPartitionRef1 = prev(all_blocks.end());

        // add that partition to set
        free_blocks.insert(newPartitionRef1);
        
        // insert the process regularly 
        // INSERTION ALGORITHM ################

        // get largest free partition
        set<PartitionRef,scmp>::iterator PR_set2 = free_blocks.begin();
        // if the same size as the process
        if((*PR_set2) -> size == size){          
          (*PR_set2) -> tag = tag;
          tagged_blocks[tag].push_back((*PR_set2));
          free_blocks.erase(PR_set2);
        }

        // if larger, 
        else{          

          PartitionRef old_Ref1 = (*PR_set2);
          old_Ref1 -> size = (*PR_set2) -> size - size;
          old_Ref1 -> address = (*PR_set2) -> address + size;

          Partition newPARTITION2 = Partition(tag, size, ((*PR_set2) -> address) - size);
          all_blocks.insert((*PR_set2), newPARTITION2);
          PartitionRef newPR2 = prev(*PR_set2);
          tagged_blocks[tag].push_back(newPR2);

          free_blocks.erase(PR_set2);
          free_blocks.insert(old_Ref1);

        }

        // ###################################

      } // END CASE: LAST PARTITION IS NOT FREE (2)
      
    }// END CASE: NEW PAGES ARE REQUIRED TO FULFILL IT     



    // CASE: NEW PAGES ARE REQUIRED TO FULFILL IT (DUPLICATE)
    else if( (*free_blocks.begin()) -> size < size ){

      int64_t pages_required = 0;

      // CASE: LAST PARTITION IS FREE (-1)
      if(prev(all_blocks.end()) -> tag < 0){    
 
        // determine number of pages required   
        int total_Req_mem = size - (prev(all_blocks.end()) -> size);

        if(total_Req_mem <= pageSize){
          pages_required = 1;
          pages_requested += 1;
        }
        
        else{          
          while(true){
            total_Req_mem -= pageSize;
            pages_required += 1;
            pages_requested += 1;
            if(total_Req_mem <= 0){
              break;
            }
          } 
        }

        // create a new free partition of size = pageSize*pages_required + the previous partition size
        Partition newPartition0 = Partition(-1, (pageSize*pages_required) + (prev(all_blocks.end()) -> size), (prev(all_blocks.end()) -> address));

        // add partition to end of all blocks
        all_blocks.insert(all_blocks.end(), newPartition0);

        // create REF to that partition
        PartitionRef newPartitionRef0 = prev(all_blocks.end());

        // add that partition to set
        free_blocks.insert(newPartitionRef0);

        // delete old partition from set and list
        free_blocks.erase(prev(newPartitionRef0));
        all_blocks.erase(prev(newPartitionRef0));
        
        // insert the process regularly 
        // INSERTION ALGORITHM ################

        // get largest free partition
        set<PartitionRef,scmp>::iterator PR_set = free_blocks.begin();
        // if the same size as the process
        if((*PR_set) -> size == size){          
          (*PR_set) -> tag = tag;
          tagged_blocks[tag].push_back((*PR_set));
          free_blocks.erase(PR_set);
        }

        // if larger
        else{          

          PartitionRef old_Ref2 = (*PR_set);
          old_Ref2 -> size = (*PR_set) -> size - size;
          old_Ref2 -> address = (*PR_set) -> address + size;

          Partition newPARTITION = Partition(tag, size, ((*PR_set) -> address) - size);
          all_blocks.insert((*PR_set), newPARTITION);
          PartitionRef newPR = prev(*PR_set);
          tagged_blocks[tag].push_back(newPR);

          free_blocks.erase(PR_set);
          free_blocks.insert(old_Ref2);

        }

        // ###################################

      }// END CASE: LAST PARTITION IS FREE (-1)


      // CASE: LAST PARTITION IS NOT FREE (2)
      else{
        // determine number of pages required   
        int total_Req_mem = size;

        if(total_Req_mem <= pageSize){
          pages_required = 1;
          pages_requested += 1;
        }
        
        else{          
          while(true){
            total_Req_mem -= pageSize;
            pages_required += 1;
            pages_requested += 1;
            if(total_Req_mem <= 0){
              break;
            }
          }
        }

        // create a new free partition of size = pageSize*pages_required
        Partition newPartition1 = Partition(-1, (pageSize*pages_required), (prev(all_blocks.end()) -> address) +  (prev(all_blocks.end()) -> size));

        // add partition to end of all blocks
        all_blocks.insert(all_blocks.end(), newPartition1);

        // create REF to that partition
        PartitionRef newPartitionRef1 = prev(all_blocks.end());

        // add that partition to set
        free_blocks.insert(newPartitionRef1);
        
        // insert the process regularly 
        // INSERTION ALGORITHM ################

        // get largest free partition
        set<PartitionRef,scmp>::iterator PR_set2 = free_blocks.begin();
        // if the same size as the process
        if((*PR_set2) -> size == size){          
          (*PR_set2) -> tag = tag;
          tagged_blocks[tag].push_back((*PR_set2));
          free_blocks.erase(PR_set2);
        }

        // if larger
        else{          

          PartitionRef old_Ref3 = (*PR_set2);
          old_Ref3 -> size = (*PR_set2) -> size - size;
          old_Ref3 -> address = (*PR_set2) -> address + size;

          Partition newPARTITION2 = Partition(tag, size, ((*PR_set2) -> address) - size);
          all_blocks.insert((*PR_set2), newPARTITION2);
          PartitionRef newPR2 = prev(*PR_set2);
          tagged_blocks[tag].push_back(newPR2);

          free_blocks.erase(PR_set2);
          free_blocks.insert(old_Ref3);
        }

        // ###################################

      } // END CASE: LAST PARTITION IS NOT FREE (2)



    }// END CASE: NEW PAGES ARE REQUIRED TO FULFILL IT (DUPLICATE)

    // CASE: NO NEW PAGES ARE REQUIRED TO FULFILL IT
    else{

      // INSERTION ALGORITHM ################

      // get largest free partition
      set<PartitionRef,scmp>::iterator PR_set3 = free_blocks.begin();
      // if the same size as the process
      if((*PR_set3) -> size == size){          
        (*PR_set3) -> tag = tag;
        tagged_blocks[tag].push_back((*PR_set3));
        free_blocks.erase(PR_set3);
      }

      // if larger
      else{          
        
        PartitionRef old_Ref4 = (*PR_set3);
        old_Ref4 -> size = (*PR_set3) -> size - size;
        old_Ref4 -> address = (*PR_set3) -> address + size;

        Partition newPARTITION3 = Partition(tag, size, ((*PR_set3) -> address) - size);
        all_blocks.insert((*PR_set3), newPARTITION3);
        PartitionRef newPR3 = prev(*PR_set3);
        tagged_blocks[tag].push_back(newPR3);

        free_blocks.erase(PR_set3);
        free_blocks.insert(old_Ref4);

      }

      // ###################################

    }// END CASE: NO NEW PAGES ARE REQUIRED TO FULFILL IT 

  }// END ALLOCATION


  void deallocate(int tag)
  {

    // CASE: TAG NOT CURRENTLY USING MEM
    if(tagged_blocks[tag].size() == 0){
      // do nothing
    }

    // CASE: TAG IS USING AT LEAST ONE PARTITION OF MEMORY
    else{

      // iterate through each tagged partition
      for(size_t i = 0; i < tagged_blocks[tag].size(); ++i){

        // CASE: IT IS SURROUNDED BY OTHER TAGGED PROCESSES, OR IS BEGINING/END WITH OTHERSIDE A TAGGED PROCESS
        if( ((all_blocks.begin() == tagged_blocks[tag][i]) & (next( tagged_blocks[tag][i])  -> tag != -1)) | ((prev( tagged_blocks[tag][i] ) -> tag != -1) & (next( tagged_blocks[tag][i])  -> tag != -1)) | ((prev(all_blocks.end()) == tagged_blocks[tag][i]) & (prev( tagged_blocks[tag][i]) -> tag != -1))){

          // change to free
          (tagged_blocks[tag][i]) -> tag = -1;

          // insert into set
          free_blocks.insert(tagged_blocks[tag][i]);          

        }// END CASE: IT IS SURROUNDED BY OTHER TAGGED PROCESSES

        // CASE: IT NEEDS TO BE MERGED WITH AT LEAST ONE FREE ADJACENT AFTER FREEING
        else{

          // CASE: IT NEEDS TO BE MERGED WITH THE LEFT
          if(all_blocks.begin() != tagged_blocks[tag][i]){
            if(prev( tagged_blocks[tag][i] ) -> tag == -1){
              // update params
              tagged_blocks[tag][i] -> size += prev( tagged_blocks[tag][i] ) -> size;
              tagged_blocks[tag][i] -> address = prev( tagged_blocks[tag][i] ) -> address;

              // delete old free block
              free_blocks.erase(prev( tagged_blocks[tag][i] ));
              all_blocks.erase(prev( tagged_blocks[tag][i] ));

            }
          }// END CASE: IT NEEDS TO BE MERGED WITH THE LEFT

          // CASE: IT NEEDS TO BE MERGED WITH THE RIGHT
          if(prev(all_blocks.end()) != tagged_blocks[tag][i]){
            if(next( tagged_blocks[tag][i] ) -> tag == -1){
              // update params
              tagged_blocks[tag][i] -> size += next( tagged_blocks[tag][i] ) -> size;

              // delete old free block
              free_blocks.erase(next(tagged_blocks[tag][i]));
              all_blocks.erase(next( tagged_blocks[tag][i] ));

            }
          }// END CASE: IT NEEDS TO BE MERGED WITH THE RIGHT

          // change to free
          (tagged_blocks[tag][i]) -> tag = -1;

          // add to set
          free_blocks.insert(tagged_blocks[tag][i]);

        }// END CASE: IT NEEDS TO BE MERGED WITH AT LEAST ONE FREE ADJACENT AFTER FREEING   

      }// END OF ITERATING THROUGH ALL PARTITIONREFS FOR TAG

      // remove all instances of tag Partitions from map
      tagged_blocks[tag].clear();

    }// END CASE: TAG IS USING AT LEAST ONE PARTITION OF MEMORY  

  }// END DEALLOCATION

  MemSimResult getStats()
  {
    
    MemSimResult result;
    // find largest partition size (will be free_blocks.begin())
    result.max_free_partition_size = (*free_blocks.begin()) -> size;
    result.max_free_partition_address = (*free_blocks.begin()) -> address;
    
    // track pages requested
    result.n_pages_requested = pages_requested;
    return result;
  }
  
};


// re-implement the following function
// ===================================
// parameters:
//    page_size: integer in range [1..1,000,000]
//    requests: array of requests
// return:
//    some statistics at the end of simulation
MemSimResult mem_sim(int64_t page_size, const std::vector<Request> & requests)
{
  // create a new simulater object
  Simulator sim(page_size);

  // set up the initial list/set/map items
  sim.setup();

  // run partitioning algorithm on requests one at a time through simulator object
  for (const auto & req : requests) {

    // if the tag is less than 0, deallocate mem for that request
    if (req.tag < 0) {
      sim.deallocate(-req.tag);

    // otherwise allocate mem for that request
    } else {
      sim.allocate(req.tag, req.size);
    }
  }
  // returns the:
  // total number of pages requested during the simulation
  // the size of the largest free partition at the end
  // the address of the largest free partition at the end
  return sim.getStats();
}
