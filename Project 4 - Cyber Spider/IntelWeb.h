#ifndef INTELWEB_H_
#define INTELWEB_H_

#include "InteractionTuple.h"
#include "DiskMultiMap.h"
#include <string>
#include <vector>

class IntelWeb
{
public:
    IntelWeb() {nameOfMapInit = nameOfMapTarget = m_filePrefix = "";}
    ~IntelWeb() {close();}
    bool createNew(const std::string& filePrefix, unsigned int maxDataItems);
    bool openExisting(const std::string& filePrefix);
    void close() {mapInitToTarget.close(); mapTargetToInit.close();}
    bool ingest(const std::string& telemetryFile);
    unsigned int crawl(const std::vector<std::string>& indicators,
                       unsigned int minPrevalenceToBeGood,
                       std::vector<std::string>& badEntitiesFound,
                       std::vector<InteractionTuple>& interactions
                       );
    bool purge(const std::string& entity);
    
    void printAllContents() {mapInitToTarget.printAll(); cout << endl; mapTargetToInit.printAll();}
    
private:
    DiskMultiMap mapInitToTarget; // keys are websites
    DiskMultiMap mapTargetToInit; // keys are programs
    string nameOfMapInit; // filename of the binary file storing MAPWEB
    string nameOfMapTarget; // filename of the binary file storing MAPEXE
    string m_filePrefix; // store the prefix passed into createNew
    
    string checkType(const string & str);
    int P(const int &minPrevalence, const std::string &value);
};


//for sorting the interaction tuples with the set
inline
bool operator<(const InteractionTuple &left, const InteractionTuple &right)
{
    if (left.context < right.context) return true;
    if (left.context > right.context) return false;
    
    //otherwise, their contexts are equal: now compare the from fields
    if (left.from < right.from) return true;
    if (left.from > right.from) return false;
    
    // now, both the context and from fields are equal: compare the to field
    return (left.to < right.to);
}

#endif // INTELWEB_H_
