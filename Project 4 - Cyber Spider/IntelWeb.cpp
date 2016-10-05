//
//  IntelWeb.cpp
//  Project 4 - Cyber Spider
//
//  Created by Yuching Cao on 3/9/16.
//  Copyright © 2016 Yuching Cao. All rights reserved.
//

#include "IntelWeb.h"
#include <string>
#include <set>
#include <algorithm>
#include <fstream>
#include <stack>

bool IntelWeb::createNew(const std::string& filePrefix, unsigned int maxDataItems)
{
    //create maps with a load factor of 1/2
    close();
    
    m_filePrefix = filePrefix;
    nameOfMapInit = filePrefix + "InitToTargetKeys.dat";
    nameOfMapTarget = filePrefix + "TargetToInitKeys.dat";
    
    bool check1 = mapInitToTarget.createNew(nameOfMapInit, maxDataItems * 2);
    bool check2 = mapTargetToInit.createNew(nameOfMapTarget, maxDataItems * 2);
    
    if (!(check1 && check2)) close();
    return check1 && check2;
}

bool IntelWeb::openExisting(const std::string &filePrefix)
{
    close();
    if (nameOfMapInit == "" || nameOfMapTarget == "") return false;
    if (m_filePrefix == "" || m_filePrefix != filePrefix) return false;
    
    bool check1 = mapInitToTarget.openExisting(nameOfMapInit);
    bool check2 = mapTargetToInit.openExisting(nameOfMapTarget);
    
    if (!(check1 && check2)) close();
    return check1 && check2;
}

string IntelWeb::checkType(const string &str)
{
    // find the index of the last element of str
    int location = str.size() - 1;
    
    //check the last 3 characters of str to see if they're "exe" or something else
    // if exe, the str is a progam, otherwise it's a website
    char third = str[location];
    char second = str[location-1];
    char first = str[location-2];
    if (first == 'e' && second == 'x' && third == 'e')
        return "exe";
    
    return "com";
}


bool IntelWeb::ingest(const std::string& telemetryFile)
{
    close();
    if (!openExisting(m_filePrefix)) return false;
    string a = ""; // will always be a computer/context
    string b = ""; // will always be the key
    string c = ""; // will always be the value
    
    //std::istringstream iss(telemetryFile);
    ifstream infile(telemetryFile);
    
    if (!infile.good()) return false;
    
    while (infile >> a >> b >> c)
    {
        // insert the tuples
        mapInitToTarget.insert(b, c, a);
        mapTargetToInit.insert(c, b, a);
    }
    return true;
}

unsigned int IntelWeb::crawl(const std::vector<std::string>& indicators,
                   unsigned int minPrevalenceToBeGood,
                   std::vector<std::string>& badEntitiesFound,
                   std::vector<InteractionTuple>& badInteractions
                   )
{
    //clear the vectors
    badEntitiesFound.clear();
    badInteractions.clear();
    
    //sets for containing the bad entities and tuples
    set<string> badEntitiesSet;
    set<InteractionTuple> badInteractionSet;
    set<string>::iterator foundInSet;
    
    //setup the objects we'll be using in the searches below
    MultiMapTuple multiTuple;
    vector<string>::iterator checkBadEntitiesFound;
 
    //stack for storing entities
    stack<string> badStack;
    
    //fill the stack with the indicators
    for (int i = 0; i != indicators.size(); i++)
    {
        badStack.push(indicators[i]);
    }
    
    while (!badStack.empty())
    {
        string top;
        top = badStack.top();
        badStack.pop();
        
        DiskMultiMap::Iterator InitToTargetIter = mapInitToTarget.search(top);
        DiskMultiMap::Iterator TargetToInitIter = mapTargetToInit.search(top);
        
        // search the forward map
        while (InitToTargetIter.isValid())
        {
            //if the current value is not in badEntitiesFound and its prevalence is below minPrev
            //then add it to badEntitiesFound
            multiTuple = (*InitToTargetIter);
            string value = multiTuple.value;
            
            //add the value to badEntitiesFound and insert the InteractionTuple
            InteractionTuple interaction;
            interaction.from = multiTuple.key;
            interaction.to = multiTuple.value;
            interaction.context = multiTuple.context;
            if (badInteractionSet.count(interaction) == 0)
                badInteractionSet.insert(interaction);
            
            //check if the value is already in the set
            foundInSet = find(badEntitiesSet.begin(), badEntitiesSet.end(), value);
            if (foundInSet != badEntitiesSet.end())
            {
                ++InitToTargetIter;
                continue;
            }
            
            if (P(minPrevalenceToBeGood, value) >= minPrevalenceToBeGood)
            {
                ++InitToTargetIter;
                continue;
            }
            
            //passed all tests: add the value to badEntitiesSet
            badEntitiesSet.insert(value);
            badStack.push(value);
            
            //badEntitiesFound.push_back(value);
            
            ++InitToTargetIter;
        }
        
        //search the backwards map
        while (TargetToInitIter.isValid())
        {
            //if the current value is not in badEntitiesFound and its prevalence is below minPrev
            //then add it to badEntitiesFound
            multiTuple = (*TargetToInitIter);
            string value = multiTuple.value;
            
            //insert the InteractionTuple
            InteractionTuple interaction;
            interaction.to = multiTuple.key; // these are reversed from the above loop
            interaction.from = multiTuple.value;
            interaction.context = multiTuple.context;
            if (badInteractionSet.count(interaction) == 0)
                badInteractionSet.insert(interaction);
            
            //check if the value is already in the set
            foundInSet = find(badEntitiesSet.begin(), badEntitiesSet.end(), value);
            if (foundInSet != badEntitiesSet.end())
            {
                ++TargetToInitIter;
                continue;
            }
            if (P(minPrevalenceToBeGood, value) >= minPrevalenceToBeGood)
            {
                ++TargetToInitIter;
                continue;
            }
            
            //passed all tests: add the value to badEntitiesSet
            badEntitiesSet.insert(value);
            badStack.push(value);
            
            ++TargetToInitIter;
        }
    }
    
    //copy badEntitiesSet into badEntitiesFound (already sorted in the set)
    set<string>::iterator copyIntoBEF = badEntitiesSet.begin();
    for (; copyIntoBEF != badEntitiesSet.end(); copyIntoBEF++)
        badEntitiesFound.push_back(*copyIntoBEF);
    
    //copy badInteractionSet into badInteractions (already sorted with operator< and the set)
    set<InteractionTuple>::iterator copyIntoBI = badInteractionSet.begin();
    for (; copyIntoBI != badInteractionSet.end(); copyIntoBI++)
        badInteractions.push_back(*copyIntoBI);
    
    return (unsigned int)badEntitiesFound.size();
}

bool IntelWeb::purge(const std::string& entity)
{
    //setup iterators for the two maps
    DiskMultiMap::Iterator InitToTargetIter = mapInitToTarget.search(entity);
    DiskMultiMap::Iterator TargetToInitIter = mapTargetToInit.search(entity);
    
    vector<string> indicator(1);
    indicator[0] = entity;
    
    int minPrevalenceToBeGood = 1000000;
    vector<string> badEntitiesFound(1000);
    vector<InteractionTuple> badInteractions(1000);
    
    crawl(indicator, minPrevalenceToBeGood, badEntitiesFound, badInteractions);
    for (int i = 0; i != badInteractions.size(); i++)
    {
        InteractionTuple t = badInteractions[i];
        if (t.from == entity || t.to == entity)
        {
            mapInitToTarget.erase(t.from, t.to, t.context);
            mapTargetToInit.erase(t.to, t.from, t.context);
        }
    }
    
    /*
    // if the entity isn't found in either of them, don't do anyhting and return false
    if (!InitToTargetIter.isValid() && !TargetToInitIter.isValid()) return false;
    
    while (InitToTargetIter.isValid())
    {
        MultiMapTuple mmt = *InitToTargetIter;
        mapInitToTarget.erase(mmt.key, mmt.value, mmt.context);
        ++InitToTargetIter;
    }
    
    while (TargetToInitIter.isValid())
    {
        MultiMapTuple mmt = *TargetToInitIter;
        mapTargetToInit.erase(mmt.key, mmt.value, mmt.context);
        ++TargetToInitIter;
    }
    */
    return true;
}
 
int IntelWeb::P(const int &minPrevalence, const std::string &value)
{
    DiskMultiMap::Iterator InitToTargetIter = mapInitToTarget.search(value);
    DiskMultiMap::Iterator TargetToInitIter = mapTargetToInit.search(value);
    int prevalenceInInitToTarget = 0;
    int prevalenceInTargetToInit = 0;
    int totalPrevalence = 0;
    
    //search forwards map
    while (InitToTargetIter.isValid())
    {
        prevalenceInInitToTarget++;
        ++InitToTargetIter;
        if (prevalenceInInitToTarget >= minPrevalence)
            return prevalenceInInitToTarget;
    }
    
    //search backwards map
    while (TargetToInitIter.isValid())
    {
        prevalenceInTargetToInit++;
        ++TargetToInitIter;
        if (prevalenceInTargetToInit >= minPrevalence)
            return prevalenceInTargetToInit;
    }
    
    totalPrevalence = prevalenceInInitToTarget + prevalenceInTargetToInit;
    return totalPrevalence;
}