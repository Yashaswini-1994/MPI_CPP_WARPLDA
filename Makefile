
DBFLGS= -DNDEBUG -g -fno-omit-frame-pointer

#WLDA
WLDAFLAGS = $(DBFLGS) -fopenmp -march=native -rdynamic -lnuma $(LIBS_DIR)/WarpLDA/release/gflags/libgflags_nothreads.a
WLDA_COMMON= $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/common.dir/AdjList.cpp.o $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/common.dir/Bigraph.cpp.o $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/common.dir/clock.cpp.o $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/common.dir/NumaArray.cpp.o $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/common.dir/Vocab.cpp.o
WLDA_WARP= $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/warplda.dir/lda.cpp.o $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/warplda.dir/warp.cpp.o $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/warplda.dir/warplda.cpp.o
WLDA_FORMAT= $(LIBS_DIR)/WarpLDA/release/src/CMakeFiles/format.dir/format.cpp.o
WLDA_INC=  -I$(LIBS_DIR)/WarpLDA/release/gflags/include/ -I$(LIBS_DIR)/WarpLDA/./src#/gflags/gflags.h WarpLDA/release/gflags/include/gflags/gflags_declare.h WarpLDA/release/gflags/include/gflags/gflags_gflags.h



# C++ compiler configuration
CXX				= mpic++
CXXFLAGS	= -O3 -Wall -std=c++11 $(WLDAFLAGS)




# Project configuration

SRC_DIR= src
LIBS_DIR= $(SRC_DIR)/LDA_Libraries

OBJS=		main.o commons.o logger.o config.o document.o cluster.o myutils.o scanner.o dataProvenance.o geneticAlgorithm.o resultStatistics.o populationConfig.o parallelizables.o TopicModelling.o preProcessing.o wordFilter.o strtokenizer.o timer.o



MAIN= provenance

all: $(OBJS) 
	$(CXX) -o $(MAIN) $(OBJS) $(WLDA_COMMON) $(WLDA_WARP) $(WLDA_FORMAT) $(WLDA_INC) ${CXXFLAGS}



main.o: $(SRC_DIR)/main.cpp
	$(CXX) -c -o main.o $(SRC_DIR)/main.cpp $(WLDA_INC) $(CXXFLAGS)

config.o: $(SRC_DIR)/config.cpp
	$(CXX) -c -o config.o $(SRC_DIR)/config.cpp $(CXXFLAGS)

commons.o: $(SRC_DIR)/commons.cpp
	$(CXX) -c -o commons.o $(SRC_DIR)/commons.cpp $(CXXFLAGS)

timer.o: $(SRC_DIR)/timer.cpp
	$(CXX) -c -o timer.o $(SRC_DIR)/timer.cpp $(CXXFLAGS)

strtokenizer.o: $(SRC_DIR)/strtokenizer.cpp
	$(CXX) -c -o strtokenizer.o $(SRC_DIR)/strtokenizer.cpp $(CXXFLAGS) $(WLDA_INC)

logger.o: $(SRC_DIR)/logger.cpp
	$(CXX) -c -o logger.o $(SRC_DIR)/logger.cpp $(CXXFLAGS)

document.o: $(SRC_DIR)/document.cpp
	$(CXX) -c -o document.o $(SRC_DIR)/document.cpp $(CXXFLAGS)

cluster.o: $(SRC_DIR)/cluster.cpp
	$(CXX) -c -o cluster.o $(SRC_DIR)/cluster.cpp $(CXXFLAGS)

myutils.o: $(SRC_DIR)/utils.cpp
	$(CXX) -c -o myutils.o $(SRC_DIR)/utils.cpp $(CXXFLAGS)

scanner.o: $(SRC_DIR)/scanner.cpp
	$(CXX) -c -o scanner.o $(SRC_DIR)/scanner.cpp $(CXXFLAGS)

dataProvenance.o: $(SRC_DIR)/dataProvenance.cpp
	$(CXX) -c -o dataProvenance.o $(SRC_DIR)/dataProvenance.cpp $(WLDA_INC) $(CXXFLAGS)

geneticAlgorithm.o: $(SRC_DIR)/geneticAlgorithm.cpp
	$(CXX) -c -o geneticAlgorithm.o $(SRC_DIR)/geneticAlgorithm.cpp $(WLDA_INC) $(CXXFLAGS)

resultStatistics.o: $(SRC_DIR)/resultStatistics.cpp
	$(CXX) -c -o resultStatistics.o $(SRC_DIR)/resultStatistics.cpp $(CXXFLAGS)

populationConfig.o: $(SRC_DIR)/populationConfig.cpp
	$(CXX) -c -o populationConfig.o $(SRC_DIR)/populationConfig.cpp $(CXXFLAGS)

parallelizables.o: $(SRC_DIR)/parallelizables.cpp
	$(CXX) -c -o parallelizables.o $(SRC_DIR)/parallelizables.cpp $(CXXFLAGS)

TopicModelling.o: $(SRC_DIR)/TopicModelling.cpp
	$(CXX) -c -o TopicModelling.o $(SRC_DIR)/TopicModelling.cpp $(CXXFLAGS) $(WLDA_INC)


preProcessing.o: $(SRC_DIR)/preProcessing.cpp
	$(CXX) -c -o preProcessing.o $(SRC_DIR)/preProcessing.cpp $(CXXFLAGS) $(WLDA_INC)

wordFilter.o: $(SRC_DIR)/wordFilter.cpp
	$(CXX) -c -o wordFilter.o $(SRC_DIR)/wordFilter.cpp $(CXXFLAGS)




clean:
	rm $(OBJS)
	rm $(GLDAOBJS)
	rm $(PLDAOBJS)
	rm $(MAIN)

