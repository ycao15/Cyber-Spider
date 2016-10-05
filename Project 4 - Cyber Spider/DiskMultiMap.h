#ifndef DISKMULTIMAP_H_
#define DISKMULTIMAP_H_

#include <string>
#include <functional>
#include <vector>
#include "BinaryFile.h"
#include "MultiMapTuple.h"

class DiskMultiMap
{
public:
    
    class Iterator
    {
    public:
        Iterator()
        {
            currentOffset = -1;
            m_tuple.key = m_tuple.value = m_tuple.context = "";
        }
        
        Iterator(vector< pair<string, string> > &Values, BinaryFile *binaryFileObject, int first, int KeyNodeLocation = -1, const char *key = "")
        {
            bf = binaryFileObject;
            firstPastBucket = first;
            
            vectorLocation = 0;
            ValueNodes = Values;
            
            currentOffset = KeyNodeLocation;
            if (isValid()) m_tuple.key = key;
            else m_tuple.key = "";
            m_tuple.value = ValueNodes[0].first;
            m_tuple.context = ValueNodes[0].second;
        }
        // You may add additional constructors
        
        bool isValid() const
        {
            //pointing in the disk:
            //if the iterator isn't pointing to the beginning of a node, it's invalid
            // if it's pointing off the end of the file, it's invalid
            int nodeOffsetFromBuckets = (currentOffset - firstPastBucket);
            if (currentOffset < firstPastBucket || nodeOffsetFromBuckets % 248 > 0) return false;
            if (currentOffset >= bf->fileLength()) return false;
            
            //pointing in the vector:
            if (vectorLocation >= ValueNodes.size()) return false;
            
            return true;
        }
        Iterator& operator++();
        MultiMapTuple operator*();
        
    private:
        BinaryFile *bf;
        int currentOffset;
        int firstPastBucket;
        MultiMapTuple m_tuple;
        int vectorLocation;
        
        pair<string, string> VandCPair;
        vector< pair<string,string> > ValueNodes;
    };
    
    DiskMultiMap() //SETUP NUMBUCKETS AND PREVALENCE
    {}
    ~DiskMultiMap() {bf.close();} // DONE
    bool createNew(const std::string& filename, unsigned int numBuckets); // TODO~
    bool openExisting(const std::string& filename) // DONE
        {
            bf.close();
            return bf.openExisting(filename);
        }
    void close() {bf.close();} // DONE
    
    bool insert(const std::string& key, const std::string& value, const std::string& context);
    Iterator search(const std::string& key);
    int erase(const std::string& key, const std::string& value, const std::string& context);
    void printAll();
    
private:
    BinaryFile bf;
    string fileName;
    int nextOpenSpace;
    int m_prevalence;
    int NUMBUCKETS;
    int firstPastBucket; // pass this into the iterator
    
    
    
    BinaryFile::Offset hash(const string &hashMe)
    {
        std::hash<string> hasher;
        unsigned long hashValue = hasher(hashMe);
        int hashLocation = hashValue % NUMBUCKETS; // which bucket number

        return ((hashLocation*4) + 8);
    }
    
    BinaryFile::Offset findInsert();
    int insertKey(const std::string &key);
    int insertValue(const std::string& value, const std::string& context);
    
    
    
    struct KeyNode // 248 Bytes - where the buckets point to
    {
        KeyNode()
        {
            nextValue = -1;
            nextKey = -1;
            keyIdentify = -10;
            numValues = 1;
        }
        BinaryFile::Offset nextValue;
        BinaryFile::Offset nextKey;
        int keyIdentify;
        char key[121];
        char filler[108]; // fill size of KeyNode to size of ValueNode
        int numValues;
    };
    
    struct ValueNode // 248 Bytes - for storing values and contexts
    {
        ValueNode() {nextValue = -1;}
        BinaryFile::Offset nextValue;
        char value[121];
        char context[121];
    };
};
#endif // DISKMULTIMAP_H_