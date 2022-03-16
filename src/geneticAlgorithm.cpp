
#include "geneticAlgorithm.h"
#include "mpi.h"

GeneticAlgorithm::GeneticAlgorithm()
{
    MAX_TOPICS = 15;
    MAX_ITERATIONS = 1000;
}

bool GeneticAlgorithm::calculateCentroids(double *clusterCentroids, multimap<int, int> *clusterMap, TopicModelling *tm, int numberOfTopics)
{
    for (int k = 0; k < numberOfTopics; k++)

    {
        int docsOfK = clusterMap->count(k);
        // topic does not have any document
        if (docsOfK <= 0)
            continue;

        std::pair<multimap<int, int>::iterator, multimap<int, int>::iterator> result = clusterMap->equal_range(k);

        // topic does not have any document
        if (result.first == result.second)
            continue;

        for (multimap<int, int>::iterator itr = result.first; itr != result.second; itr++)
        {
            for (int t = 0; t < numberOfTopics; t++)
            {
                clusterCentroids[(k * numberOfTopics) + t] = clusterCentroids[(k * numberOfTopics) + t] + tm->getDistribution(t, itr->second);
            }
        }
        for (int t = 0; t < numberOfTopics; t++)
        {
            clusterCentroids[(k * numberOfTopics) + t] = clusterCentroids[(k * numberOfTopics) + t] / docsOfK;
        }
    }
    return true;
}

// finding the distance of each documents in each cluster
// finding max distance from other documents in the same cluster
bool GeneticAlgorithm::getMaxDistancesInsideClusters(double *maxDistanceInsideCluster,
                                                     multimap<int, int> *clusterMap,
                                                     TopicModelling *tm,
                                                     int numberOfTopics)
{

    for (int k = 0; k < numberOfTopics; k++)
    {
        int docsOfK = clusterMap->count(k);
        // topic does not have any document
        if (docsOfK <= 0)
            continue;

        int counter = docsOfK;
        for (multimap<int, int>::iterator itr = clusterMap->find(k); counter > 0; itr++, counter--)
        {
            int docNo = itr->second;
            maxDistanceInsideCluster[docNo] = 0;

            int counter2 = docsOfK;
            for (multimap<int, int>::iterator itr2 = clusterMap->find(k); counter2 > 0; itr2++, counter2--)
            {
                int otherDocNo = itr2->second;
                if (otherDocNo == docNo)
                {
                    continue;
                }

                // finding euclidean distance between the two points/docuemnts
                double distance = 0;
                for (int h = 0; h < numberOfTopics; h++)
                {
                    distance += pow((tm->getDistribution(h, otherDocNo) - tm->getDistribution(h, docNo)), 2);
                }
                distance = sqrt(distance);
                if (distance > maxDistanceInsideCluster[docNo])
                {
                    maxDistanceInsideCluster[docNo] = distance;
                }
            }
        }
    }

    return true;
}

// finding each documents minimum distance to the centroids of other clusters
bool GeneticAlgorithm::getMinDistancesOutsideClusters(double *minDistanceOutsideCluster,
                                                      multimap<int, int> *clusterMap,
                                                      TopicModelling *tm,
                                                      int numberOfTopics, ConfigOptions *cfg)
{
    // first calculates centroids
    double *clusterCentroids = new double[numberOfTopics * numberOfTopics]{};
    if (!calculateCentroids(clusterCentroids, clusterMap, tm, numberOfTopics))
        cfg->logger.log(error, "Error getting centroids");

    for (int k = 0; k < numberOfTopics; k++)
    {
        int docsOfK = clusterMap->count(k);
        // topic does not have any document
        if (docsOfK <= 0)
            continue;

        int counter = docsOfK;
        for (multimap<int, int>::iterator itr = clusterMap->find(k); counter > 0; itr++, counter--)
        {
            int docNo = itr->second;
            minDistanceOutsideCluster[docNo] = INT_MAX;
            for (int z = 0; z < numberOfTopics; z++)
            {
                // don't calculate the distance to the same cluster
                if (z == k)
                {
                    continue;
                }
                double distance = 0;
                for (int h = 0; h < numberOfTopics; h++)
                {
                    distance += pow((clusterCentroids[(z * numberOfTopics) + h] - tm->getDistribution(h, docNo)), 2);
                }
                distance = sqrt(distance);
                if (distance < minDistanceOutsideCluster[docNo])
                {
                    minDistanceOutsideCluster[docNo] = distance;
                }
            }
        }
    }
    delete[] clusterCentroids;
    return true;
}

double GeneticAlgorithm::calculateFitness(TopicModelling *tm, int numberOfTopics, int numberOfDocuments, ConfigOptions *cfg)
{
    multimap<int, int> clusterMap;
    int mainTopic;
    int topicLess = 0;

    cfg->logger.log(debug, "Calculating fitness");
    // cluster docs by main topic to calculate fitness
    for (int d = 0; d < numberOfDocuments; d++)
    {
        mainTopic = tm->getMainTopic(d);
       
        if (mainTopic >= 0)
            clusterMap.insert(pair<int, int>(mainTopic, d));
        else
        {
            topicLess++;
            cfg->logger.log(error, "Document " + tm->getDocNameByNumber(d) + " is topicless.");
        }
    }
    // TODO: this is a test
    cfg->logger.log(debug, "Test Main topic");
    tm->getMainTopic(0);
    cfg->logger.log(debug, "Test Distribution");
    tm->getDistribution(0, 0);
    

    if (topicLess > 0)
        cfg->logger.log(info, "Documents without topic: " + std::to_string(topicLess) + "/" + std::to_string(numberOfDocuments));
    cfg->logger.log(debug, "Got main topics");

    double *maxDistanceInsideCluster = new double[numberOfDocuments]{};

    if (!getMaxDistancesInsideClusters(maxDistanceInsideCluster, &clusterMap, tm, numberOfTopics))
        cfg->logger.log(error, "Error getting distances inside cluster");

    double *minDistanceOutsideCluster = new double[numberOfDocuments]{};
    if (!getMinDistancesOutsideClusters(minDistanceOutsideCluster, &clusterMap, tm, numberOfTopics, cfg))
        cfg->logger.log(error, "Error getting distances outside cluster");

    cfg->logger.log(debug, "Got all data from TM");

    // calculate the Silhouette coefficient for each document
    double *silhouetteCoefficient = new double[numberOfDocuments]{};
    for (int m = 0; m < (numberOfDocuments); m++)
    {
        if (max(minDistanceOutsideCluster[m], maxDistanceInsideCluster[m]) == 0)
            silhouetteCoefficient[m] = 0;
        else
            silhouetteCoefficient[m] = (minDistanceOutsideCluster[m] - maxDistanceInsideCluster[m]) / max(minDistanceOutsideCluster[m], maxDistanceInsideCluster[m]);
    }

    // find the average of the Silhouette coefficient of all the documents - fitness criteria
    double total = 0;
    for (int m = 0; m < (numberOfDocuments); m++)
    {
        total += silhouetteCoefficient[m];
    }
    
    cfg->logger.log(debug, "Done with silhouette calculations");

    delete[] maxDistanceInsideCluster;
    delete[] minDistanceOutsideCluster;
    delete[] silhouetteCoefficient;

    cfg->logger.log(debug, "Cleaned pointers");
    
    return (total / (numberOfDocuments));
}

PopulationConfig *GeneticAlgorithm::mutateToNewPopulation(PopulationConfig *population, ConfigOptions *cfg)
{
    // ranking and ordering the chromosomes based on the fitness function.
    // We need only the top 1/3rd of the chromosomes with high fitness values - Silhouette coefficient
    PopulationConfig *newPopulation = new PopulationConfig[cfg->populationSize];
    int spanSize = cfg->populationSize / 3;

    // copy the top 1/3rd of the chromosomes to the new population
    for (int i = 0; i < spanSize; i++)
    {
        double maxFitness = -1;
        int maxFitnessChromosome = -1;

        // gets maxFitness
        for (int j = 0; j < cfg->populationSize; j++)
        {
            if (population[j].fitness_value > maxFitness)
            {
                maxFitness = population[j].fitness_value;
                maxFitnessChromosome = j;
            }
        }

        // if(i==0 && maxFitness < 0) {
        //  if best is this low, restart from scratch
        //  only test the original max
        //  apparently this was my idea, makes it converge faster
        //     for (int q = 0; q<spanSize; q++){
        //         newPopulation[q].set_max(MAX_TOPICS, MAX_ITERATIONS);
        //         newPopulation[q].random();
        //     }
        //     break;
        // }

        // copy the chromosome with high fitness to the next generation
        newPopulation[i].copy(population[maxFitnessChromosome]);
        newPopulation[i].seed = time(NULL);
        population[maxFitnessChromosome].fitness_value = INT_MIN;
    }

    // perform crossover - to fill the rest of the 2/3rd of the initial Population
    for (int i = 0; i < spanSize; i++)
    {

        newPopulation[spanSize + i].set_max(this->MAX_TOPICS, this->MAX_ITERATIONS);
        newPopulation[2 * spanSize + i].set_max(this->MAX_TOPICS, this->MAX_ITERATIONS);

        newPopulation[spanSize + i].number_of_topics = newPopulation[i].number_of_topics;
        newPopulation[spanSize + i].number_of_iterations = newPopulation[i].number_of_iterations;
        if (getRandomFloat() < cfg->mutationLevel)
            newPopulation[spanSize + i].random_topic();
        if (getRandomFloat() < cfg->mutationLevel)
            newPopulation[spanSize + i].random_iteration();

        newPopulation[2 * spanSize + i].number_of_topics = newPopulation[i].number_of_topics;
        newPopulation[2 * spanSize + i].number_of_iterations = newPopulation[i].number_of_iterations;
        if (getRandomFloat() < cfg->mutationLevel)
            newPopulation[2 * spanSize + i].random_topic();
        if (getRandomFloat() < cfg->mutationLevel)
            newPopulation[2 * spanSize + i].random_iteration();
    }

    for (int i = (3 * spanSize); i < cfg->populationSize; i++)
    {
        newPopulation[i].random_topic();
        newPopulation[i].random_iteration();
    }
    return newPopulation;
}

int getMaxFit(PopulationConfig *population, ConfigOptions *cfg)
{
    double maxFitness = -1;
    int maxFitnessChromosome = -1;

    // gets maxFitness
    for (int j = 0; j < cfg->populationSize; j++)
    {
        if (population[j].fitness_value > maxFitness)
        {
            maxFitness = population[j].fitness_value;
            maxFitnessChromosome = j;
        }
    }

    return maxFitnessChromosome;
}

double getDifference(PopulationConfig *a, PopulationConfig *b)
{
    if (a->fitness_value > b->fitness_value)
        return (a->fitness_value - b->fitness_value);

    return (b->fitness_value - a->fitness_value);
}

ResultStatistics GeneticAlgorithm::geneticLogic(int numberOfDocuments, ConfigOptions *cfg, int mpi_size, int myrank)
{

    int msize = mpi_size;
    int mrank = myrank;
    MPI_Status status;

    // cfg->logger.log(status, "inside geneticLogic");
    ResultStatistics result;
    // MAX_TOPICS = numberOfDocuments/2;
    // MAX_ITERATIONS = numberOfDocuments*10;
    PopulationConfig *population = new PopulationConfig[cfg->populationSize];
    PopulationConfig currBestConfig(population[0]); // starts on the first config
    currBestConfig.fitness_value = -1;

    int maxIddle = 5;
    int iddleIterations = 0;

    /// cfg->logger.log(status, "Starting GA with " + std::to_string(numberOfDocuments) + " files");
    // cfg->logger.log(status, "###########################");
   
    // initialize population
    for (int i = 0; i < cfg->populationSize; i++)
    {
        population[i].set_max(this->MAX_TOPICS, this->MAX_ITERATIONS); // define max of topics and iterations
        population[i].random();                                        // generate random number of topics and iterations
    }

    int GACounter = 0;
    int LDACounter = 0;
    long LDATotTime = 0;
    bool fitnessThresholdFound = false;
    int cb = 0;

    Timer t;

    int stripe = cfg->populationSize / msize;
    

    int start = mrank * stripe;
     
    int end = (mrank * stripe) + stripe - 1;
    

    int mod = cfg->populationSize % msize;

    TopicModelling *tm;
    int cbh[msize - 1];
     Timer f;
     long totaltime = 0;
    // pair<int, double>* res;
    // pair<int, double>* resresv;

    // add a limit of 1000 GA iterations, it should not run infinitly
    while (GACounter < 500)
    {
        GACounter++;
        cfg->logger.log(info, "GA Attempt: " + std::to_string(GACounter));

        if(mrank==0)
            f.start();
        for (int m = start; m <= end; m++)
        {
            string filename = "wdistribution" + to_string(m);
            
            tm = new TopicModelling(population[m].number_of_topics, population[m].number_of_iterations, numberOfDocuments, population[m].seed, cfg, filename);
            string tempFileID = "__" + to_string(m) + "__" + to_string(population[m].number_of_topics) + "x" + to_string(population[m].number_of_topics);

            long ldaTime = tm->LDA(tempFileID);
            
            // calculates fitness value to determine wether to stop or try next pair
            population[m].fitness_value = calculateFitness(tm, population[m].number_of_topics, numberOfDocuments, cfg);
           
            if (mrank > 0)
            {
            	
                MPI_Send(&m, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
                MPI_Send(&population[m].fitness_value, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
            }
            
            if (mrank == 0)
            {

                for (int dest = 1; dest < msize; dest++)
                {

                    int x1;
                    double y1;
                    MPI_Recv(&x1, 1, MPI_INT, dest, 1, MPI_COMM_WORLD, &status);
                    MPI_Recv(&y1, 1, MPI_DOUBLE, dest, 1, MPI_COMM_WORLD, &status);
                    population[x1].fitness_value = y1;
                    
                }
            }
            
            if (UNLIKELY(population[m].fitness_value >= cfg->fitnessThreshold))
            {
                string fileid = "__" + to_string(m) + "__" + to_string(population[m].number_of_topics) + "x" + to_string(population[m].number_of_topics);

                tm->WriteFiles(true, fileid);

                result.cfg.copy(population[m]);

                fitnessThresholdFound = true;

                cb = 1;

            }
			
                for (int b = 0; b < msize; b++)
                {
                if (mrank != b)
                MPI_Send(&cb, 1, MPI_INT, b, 1, MPI_COMM_WORLD);
                }

                
                for (int b = 0; b < msize; b++)
                {
                if (mrank != b)
                MPI_Recv(&cbh[b], 1, MPI_INT, b, 1, MPI_COMM_WORLD, &status);
                }

               if (cb == 1 || cbh[0] == 1 || cbh[1] == 1 || cbh[2] == 1) {
                    fitnessThresholdFound = true;
                    break;
               }

             delete tm;

            
        }

        if(mrank==0){
            long tf = f.getTime();
            
            totaltime+=tf;

        }

         int modrecv =0;
        if(mod!=0 && mrank == 0) {
            string filename = "wdistribution" + to_string(8);
            
            tm = new TopicModelling(population[8].number_of_topics, population[8].number_of_iterations, numberOfDocuments, population[8].seed, cfg, filename);
            string tempFileID = "__" + to_string(8) + "__" + to_string(population[8].number_of_topics) + "x" + to_string(population[8].number_of_topics);

            long ldaTime = tm->LDA(tempFileID);

            // calculates fitness value to determine wether to stop or try next pair
            population[8].fitness_value = calculateFitness(tm, population[8].number_of_topics, numberOfDocuments, cfg);
            delete tm;
            
            if (UNLIKELY(population[8].fitness_value >= cfg->fitnessThreshold))
            {
                string fileid = "__" + to_string(8) + "__" + to_string(population[8].number_of_topics) + "x" + to_string(population[8].number_of_topics);

                tm->WriteFiles(true, fileid);

                result.cfg.copy(population[8]);

                fitnessThresholdFound = true;

                modrecv = 1;


            }


            for (int b = 0; b < msize; b++)
                {
                if (mrank != b)
                MPI_Send(&modrecv, 1, MPI_INT, b, 1, MPI_COMM_WORLD);
                }

            


        }

        if(mod!=0 && mrank>0) {
           
            if(mrank>0) 
                 MPI_Recv(&modrecv, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);

        }
        

         if (cb == 1 || cbh[0] == 1 || cbh[1] == 1 || modrecv==1 || cbh[2] == 1) {
                    fitnessThresholdFound = true;
                    break;
               }

        // stops GA as Fitness Threshold was reached
        if(fitnessThresholdFound) break;

        if (mrank == 0)
        {

            int bestConfig = getMaxFit(population, cfg);
            
            if (population[bestConfig].fitness_value > currBestConfig.fitness_value)
            {
                currBestConfig.copy(population[bestConfig]);
                iddleIterations = 0;
            }
            else
                iddleIterations++;

            if (iddleIterations > maxIddle)
            {
                cfg->logger.log(debug, "Re-run LDA");
                string f = "wdistribution" + to_string(bestConfig);
               
                TopicModelling *tmr = new TopicModelling(currBestConfig.number_of_topics, currBestConfig.number_of_iterations, numberOfDocuments, currBestConfig.seed, cfg, f);
                tmr->LDA("");
                cfg->logger.log(debug, "Ran LDA");

                tmr->WriteFiles(true, "");
                cfg->logger.log(debug, "Wrote files");

                cfg->logger.log(info, "the best distribution is " + to_string(currBestConfig.number_of_topics) + " topics and " + to_string(currBestConfig.number_of_iterations) + " iterations and fitness is " + to_string(currBestConfig.fitness_value));

                result.cfg.copy(currBestConfig);
                cfg->logger.log(debug, "Copied population");

                cb = 1;
                // send it all other ranks canbreak
            }

            for (int b = 1; b < msize; b++)
            {
                MPI_Send(&cb, 1, MPI_INT, b, 1, MPI_COMM_WORLD);
            }
        }

        int cd;
        if (mrank > 0)
        {
            // recv by rank zero the value can break
            MPI_Recv(&cd, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
        }

        if (cd == 1 || cb==1)
            fitnessThresholdFound = true;

        // stops GA as Fitness Threshold was reached
        if (fitnessThresholdFound)
            break;

        if (mrank == 0)
        {
            cfg->logger.log(info, "Best LDA Attempt: [" + to_string(currBestConfig.number_of_topics) + "x" + to_string(currBestConfig.number_of_iterations) + "] - Fitness: " + to_string(currBestConfig.fitness_value));

            // t.start();

            PopulationConfig *newPopulation = mutateToNewPopulation(population, cfg);
            // substitute the initial population with the new population and continue

            for (int d = 1; d < msize; d++)
            {
                for (int b = 0; b < cfg->populationSize; b++)
                {
                    MPI_Send(&newPopulation[b].seed, 1, MPI_LONG, d, 1, MPI_COMM_WORLD);
                    // MPI_Send(&newPopulation[b].fitness_value, 1, MPI_DOUBLE, d, 1, MPI_COMM_WORLD);
                    MPI_Send(&newPopulation[b].number_of_iterations, 1, MPI_INT, d, 1, MPI_COMM_WORLD);
                    MPI_Send(&newPopulation[b].number_of_topics, 1, MPI_INT, d, 1, MPI_COMM_WORLD);
                }
            }

            delete[] population;
            population = newPopulation;
            //delete[] newPopulation;

            result.GA_count = GACounter;
            result.LDA_count = LDACounter;
            result.LDA_time = LDATotTime;
        }

        if (mrank > 0)
        {
            PopulationConfig *newPopulation1 = new PopulationConfig[cfg->populationSize];

            for (int b = 0; b < cfg->populationSize; b++)
            {
                MPI_Recv(&newPopulation1[b].seed, 1, MPI_LONG, 0, 1, MPI_COMM_WORLD, &status);
                // MPI_Recv(&newPopulation1[b].fitness_value, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD,&status);
                MPI_Recv(&newPopulation1[b].number_of_iterations, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
                MPI_Recv(&newPopulation1[b].number_of_topics, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
                newPopulation1[b].set_max(this->MAX_TOPICS, this->MAX_ITERATIONS);
            }

            delete[] population;
            population = newPopulation1;
           // delete[] newPopulation1;
        }
    }

    
    if(mrank==0) {
    long avg = totaltime/GACounter;
    
    cout<<"LDA AVERAGE TIME - After Paralelization  " << avg << endl;
    }

    return result;
}



void GeneticAlgorithm::sortInitialPopulation(PopulationConfig *mInitialPopulation, int size)
{
    if (mInitialPopulation == NULL || size <= 0)
    {
        return;
    }

    PopulationConfig *sortedPopulation = new PopulationConfig[size];
    for (int i = 0, j = size; i < size && j > 0; i++)
    {
        if (mInitialPopulation[i].fitness_value > 0.0f)
        {
            sortedPopulation[i - (size - j)] = mInitialPopulation[i];
        }
        else
        {
            sortedPopulation[--j] = mInitialPopulation[i];
        }
    }
    delete[] mInitialPopulation;
    mInitialPopulation = sortedPopulation;
}
