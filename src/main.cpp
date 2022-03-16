#include <iostream>
#include <time.h>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include "document.h"
#include "utils.h"
#include "dataProvenance.h"
#include "preProcessing.h"
#include "config.h"
#include "mpi.h"

using namespace std;



//void CheckLDAPerformance(int numberOfDocuments, ConfigOptions* cfg);

int main(int argc, char* argv[]) {
    int my_rank = 0;  // used by MPI
    int mpi_size = 1; // used by MPI
    MPI_Status status;
    // fixing seed for testing purposes
    srand(time(NULL));
    string configFile = "";
    int docsize;
    


    MPI_Init(&argc, &argv); // start MPI
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);

   
    // should be multiple of 3
    int arg=1;
    if(argc >= 2){
        configFile = argv[arg++];
    }
    ConfigOptions cfg(configFile);

  

    for (; arg < argc; arg++) {
        string s = argv[arg];
        if(s.compare("-p") == 0)
            cfg.populationSize = stoi(argv[++arg]);
        else if(s.compare("-m")==0)
            cfg.mutationLevel = stod(argv[++arg]);
	else if(s.compare("-f") == 0)
            cfg.fitnessThreshold = stod(argv[++arg]);
        else if(s.compare("-dir")==0){
	    cfg.inputDir = argv[++arg];
	}
        else if(s.compare("-out")==0){
	    string outdir = argv[++arg];
            cfg.outputDir = cfg.outputDir + "/" + outdir;
	}
        else if(s.compare("-truth")==0){
            cfg.truthFile = argv[++arg];
        }
        else if(s.compare("-prep")==0){
            cfg.preProcessedFile = argv[++arg];
        }
        else if(s.compare("-input")==0){
            cfg.ldaInputFile = argv[++arg];
        }
        else if(s.compare("-seed") == 0)
            srand(stoi(argv[++arg]));
        else if(s.compare("-cuda") == 0)
            cfg.perfType = cuda;
        
        else if(s.compare("-metrics") == 0)
            cfg.runType = metric;
        else if(s.compare("-debug") == 0)
            cfg.logLevel = debug;
        else if(s.compare("-info") == 0)
          cfg.logLevel = info;
        else if(s.compare("-wlda") == 0)
          cfg.ldaLibrary = wlda;
        else
            cout<<"\tparameter not recognized: "<<argv[arg]<<endl;
    }
    unordered_map<string, Document> documentsMap;


    cfg.start();

     if(my_rank==0) {
    
    documentsMap = prepareData(&cfg);
    docsize = documentsMap.size();

  
    for(int  d = 1; d< mpi_size; d++) {
         MPI_Send(&docsize, 1, MPI_INT, d,1,MPI_COMM_WORLD);
    }

  

    int articlesCount = 0;
    for (std::pair<std::string, Document> element : documentsMap)
    {
        if (element.first.find("$AAA$") != string::npos) articlesCount++;
    }
  

    cout<<"Number of Articles: "<< articlesCount << endl;
    cout<<"Number of Documents: "<<docsize << endl;
    cout<<"MPI size : " << mpi_size << endl;

    }

        if(my_rank >0) {
           
     MPI_Recv(&docsize, 1, MPI_INT, 0, 1, MPI_COMM_WORLD,&status);
    
 }
    
    if(cfg.runType == metric) {
     // cfg.logger.log(status, "Starting LDA performance test");
      //CheckLDAPerformance(docsize, &cfg);
    }
    else {
      cfg.logger.log(info, "Starting provenance");
    //cout<<docsize<<endl;

      // call genetic logic to perform LDA-GA
      reconstructProvenance(docsize, &cfg, mpi_size, my_rank);

    }

    MPI_Finalize( ); 
}

/*void CheckLDAPerformance(int numberOfDocuments, ConfigOptions *cfg) {
    int TEST_COUNT = 3;
    long LDATotTime = 0;
    string line;
    int tpcs[] = {2, 4, 6, 8, 10};
    int times[5][5];

    stringstream ss;
    for (int i = 0; i < 5; i++) {
        int number_of_topics = tpcs[i];
		for (int j = 0; j <5; j++) {
            int number_of_iterations = (j+1)*100;
            PopulationConfig popCfg(time(NULL));
            popCfg.number_of_topics = number_of_topics;
            popCfg.number_of_iterations = number_of_iterations;
            LDATotTime = 0;

            for (int i = 0; i < TEST_COUNT; ++i) {
                TopicModelling tm(number_of_topics, number_of_iterations, numberOfDocuments, popCfg.seed, cfg);
                string id = "__"+to_string(i/2)+"__"+to_string(number_of_topics)+"x"+to_string(number_of_iterations);

                LDATotTime += tm.LDA(id);
            }
            popCfg.LDA_execution_milliseconds = ((double)LDATotTime/TEST_COUNT);
            times[i][j] = popCfg.LDA_execution_milliseconds;
            ss<<number_of_topics<<"x"<<number_of_iterations<<": " + to_string(popCfg.LDA_execution_milliseconds)<<"ms";
            //cfg->logger.log(info, ss.str());
            ss.str(std::string());
            ss.clear();
        }
    }

    ss<<"topics\t";
    for(int i=100; i<=500; i+=100){
      ss<<i<<(i==500 ? "\n" : "\t");
    }
    for(int i=0; i<5; i++){
      ss<<tpcs[i]<<"\t";
      for(int j=0; j<5; j++){
        ss<<times[i][j]<<(j==4 ? "\n":"\t");
      }
    }

    //cfg->logger.log(status, ss.str());
    ss.str(std::string());
    ss.clear();
}*/
