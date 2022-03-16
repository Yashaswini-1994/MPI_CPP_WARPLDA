#include "TopicModelling.h"

using namespace std;

//#####################################################
//############## General Functions ####################
//#####################################################

TopicModelling::TopicModelling(int numberOfTopics, int numberOfIterations, int numberOfDocuments, long seed, ConfigOptions* cfg, string filename){
      this->numberOfTopics = numberOfTopics;
      this->numberOfIterations = numberOfIterations;
      this->seed = seed;
      this->numberOfDocuments = numberOfDocuments;
      this->cfg = cfg;
      this->filename = filename;
      this->doc_index_map = new vector<string>();
      this->doc_index_map->reserve(numberOfDocuments);
      LoadDocMap();
      wldaDocTopDist = new double*[numberOfDocuments];
      for(unsigned i = 0; i < numberOfDocuments; ++i)
          wldaDocTopDist[i] = new double[numberOfTopics];
  }

TopicModelling::~TopicModelling(){
      doc_index_map->clear();
          doc_index_map->shrink_to_fit();
          delete doc_index_map;
          for(int i = 0; i < numberOfDocuments; ++i)
            delete [] wldaDocTopDist[i];
          delete [] wldaDocTopDist;
  }

int TopicModelling::getMainTopic(int docNum) {
  
      double max = 0.0;
      int idMax = -1;

      for(int i = 0; i<numberOfTopics; i++) {
          double curr = getDistribution (i, docNum);
        if(curr > max) {
            max = curr;
            idMax = i;
        }
      }
      return idMax;
  
}

double TopicModelling::getDistribution(int topic, int docNum){
      
            return wldaDocTopDist[docNum][topic];
          
  }

 string TopicModelling::getDocNameByNumber(int num){
      
            return (*doc_index_map)[num];
      
  }
  void TopicModelling::WriteFiles(bool isfinal, string fileid) {
      
            return WLDA_WriteFiles(isfinal, fileid);
          
  }

  long TopicModelling::LDA(string MyCount) {
     
            return WLDA_LDA(MyCount);
         
      

      return 0;
  }

  bool TopicModelling::LoadDocMap(){
      cfg->logger.log(debug, "Reading Doc Map: " + cfg->ldaInputFile);
      string line;
      int docID = 0;
      ifstream ldaInputFile;
      ldaInputFile.open (cfg->ldaInputFile);
      while (getline(ldaInputFile, line)) {
          // Each line has top 20 words of a topic
          string name = line.substr(0, line.find(cfg->delimiter));
//          (*doc_index_map)[docID++] = name;
          doc_index_map->push_back(name);
      }
      ldaInputFile.close();

      return true;
}



// #######################################################
// ###################### WarpLDA ########################
// #######################################################
long TopicModelling::WLDA_LDA(string MyCount) {
  srand(seed);
  cfg->logger.log(debug, "#### Starting LDA with " + to_string(numberOfTopics)
                + " topics and " + to_string(numberOfIterations) + " iterations ####");

  outputFile = MyCount;

  Timer t;
  t.start();
  string winf = cfg->inputDir + "wlda_input";
  string ntopics = to_string(numberOfTopics);
  string ff = cfg->outputDir + "/"+filename ;
  
  string niters = to_string(numberOfIterations);
  char* args[] = {(char*)"warplda",
                (char*)"--prefix", (char*)winf.c_str(),
                (char*)"--k", (char*)ntopics.c_str(),
                (char*)"--niter", (char*)niters.c_str(),
                (char*)"--dir", (char*)(ff.c_str()),
                 NULL};

  cfg->logger.log(debug, "WLDA setup completed. Starting estimate");

  run_wlda(9, args);

  cfg->logger.log(debug, "WLDA estimate completed");

  // load doc x topic distribution from prefix.dist files
  if(!WLDA_loadDistributions())
    cfg->logger.log(error, "Error reading Doc x Topic distributions");

  // load words from prefix.info.words.txt file
  vector<string> topWords;
  topWords.reserve(numberOfTopics);
  if(!WLDA_loadWords(&topWords))
    cfg->logger.log(error, "Error reading TopWords for topics");

  vector<double> topDist;
  topDist.reserve(numberOfTopics);
  // TODO: load topic distribution somehow
  for (int topic = 0; topic < numberOfTopics; topic++) {
      topDist.push_back(0.0);
  }

  // write topic.txt
  ofstream outTop;
  outTop.open (ff + "/topic.txt");
  outTop<<"topic\tdist\twords"<<endl;
  for (int topic = 0; topic < numberOfTopics; topic++) {
      outTop<<topic<<"\t"<<topDist[topic]<<"\t"<<topWords[topic]<<endl;
  }

  topDist.clear();

  outTop.close();
  long time = t.getTime();

  cfg->logger.log(debug, "#### Ending LDA ####");

  return time;
}

bool TopicModelling::WLDA_loadDistributions(){
    Scanner sc2;
    try {

      
        sc2.open(cfg->outputDir + "/" +filename + "/wdistribution.txt");
         
        do {
            int docID = sc2.nextInt();
            for (unsigned t = 0; t<numberOfTopics; t++){
                int topic  = sc2.nextInt();
                double dist = sc2.nextDouble();
                wldaDocTopDist[docID][topic] = dist;
            }
        } while(sc2.nextLine());
    } catch (exception& e) {

        cfg->logger.log(error, "Hit error while reading the Distribution File");
        cfg->logger.log(error, e.what());
        return false;
    }
    sc2.close();
    
    return true;
}

bool TopicModelling::WLDA_loadWords(vector<string>* topWords){
    string line;
    try {
        ifstream topWordsFile;
        topWordsFile.open (cfg->outputDir+ + "/"+filename + "/top_words.txt");
        while (getline(topWordsFile, line)) {
            // Each line has top 20 words of a topic
            topWords->push_back(line);
        }
        topWordsFile.close();
    } catch (exception& e) {
        cfg->logger.log(error, "Hit error while reading the Distribution File");
        cfg->logger.log(error, e.what());
        return false;
    }

    return true;
}

void TopicModelling::WLDA_WriteFiles(bool isfinal, string fileid) {
    outputFile = fileid;
    ifstream idFile(cfg->outputDir + "/"+filename + "/wdistribution.txt");
   
    ofstream distFile(cfg->outputDir + "/distribution" + (isfinal ? "" : outputFile) + ".txt");
    string line;
    while (getline(idFile, line)) {
        int pos = line.find("\t");
        int docID = stoi(line.substr(0, pos));
        string docName = getDocNameByNumber(docID);
        distFile<<docName<<line.substr(pos)<<endl;
    }
    idFile.close();
    distFile.close();
}
