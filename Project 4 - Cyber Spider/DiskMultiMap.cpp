//
//  DiskMultiMap.cpp
//  Project 4 - Cyber Spider
//
//  Created by Yuching Cao on 3/5/16.
//  Copyright © 2016 Yuching Cao. All rights reserved.
//

#include "DiskMultiMap.h"

bool DiskMultiMap::createNew(const std::string &filename, unsigned int numBuckets)
{
    bf.close();
    NUMBUCKETS = numBuckets;
    
    // first item of header: number of buckets
    fileName = filename;
    bf.createNew(filename);
    bf.write(numBuckets, 0);
    
    //second item of header: head of deletion list
    bf.write(-1, bf.fileLength());
    
    int bucketStart = bf.fileLength();
    
    // O(numBuckets)
    int i;
    for (i = bucketStart; i != bucketStart + (numBuckets * 4); i += 4)
        bf.write(-1, i);
    
    
    //set the value of the offset that is the first byte past the bucket
    // i still increments after the last call of the for loop, the for loop just doesn't get called again
    firstPastBucket = i;
    
    // TODO: find cases under which this might return false
    return true;
}

bool DiskMultiMap::insert(const std::string &key, const std::string &value, const std::string &context)
{
    if (key.size() > 120 || value.size() > 120 || context.size() > 120)
        return false;
    
    close();
    openExisting(fileName);
    
    //if location in the array of buckets at the hashed value is -1, then create a new bucket and keynode
    // otherwise add it on as a collision wherever it is
    int bucket = hash(key);
    
    //create the ValueNode
    int newValueNodeLocation = insertValue(value, context);
    
    //check if the bucket has already been taken
    BinaryFile::Offset ActualBucketValue;
    bf.read(ActualBucketValue, bucket); // CHECK: could just do read(bucket, bucket)

    if (ActualBucketValue == -1) // there's no bucket here, point it to a new node
    {
        int newKeyNodeLocation = insertKey(key); // create a new KeyNode
        bf.write(newKeyNodeLocation, bucket); // link the KeyNode to the bucket
        
        //link the ValueNode to the KeyNode
        bf.write(newValueNodeLocation, newKeyNodeLocation);
        // the number of associations (number of valuenodes linked) is defaulted to one here
        return true;
    }
    
    else // a bucket has already been taken:
        // 1. check for KeyNode collisions
        // 2. navigate to the end of the linked list
    {
        //navigate to the end of an existing ValueNode, link the nodes together
        
        int nextValue;
        bf.read(nextValue, ActualBucketValue); // read address of first ValueNode into nextValue
        
        while (nextValue != -1)
        {
            ActualBucketValue = nextValue;
            bf.read(nextValue, ActualBucketValue);
        }
        bf.write(newValueNodeLocation, ActualBucketValue);
        
        int n;
        bf.read(n, ActualBucketValue);
        //update the nextValue pointer of the previous ValueNode to the address of the new ValueNode
        
    }
    return true;
}

BinaryFile::Offset DiskMultiMap::findInsert()
{
    bf.close();
    bf.openExisting(fileName);
    int insertHead;
    bf.read(insertHead, 4);
    if (insertHead == -1) return bf.fileLength();
    
    //otherwise, we need to relink the nodes
    int nextAvailable;
    bf.read(nextAvailable, insertHead);
    bf.write(nextAvailable, 4);
    
    return insertHead;
}

DiskMultiMap::Iterator DiskMultiMap::search(const std::string &key)
{
    if (!bf.isOpen())
    {
        Iterator it;
        return it;
    }
    
    int bucket = hash(key);
    int newBucket;
    bf.read(newBucket, bucket); // now we have the address of the KeyNode
    
    //if there's an empty bucket, return the default invalid iterator
    if (newBucket == -1)
    {
        Iterator it;
        return it;
    }
    
    //bucket is now the address of the KeyNode
    //read the key from the KeyNode for storage into the iterator
    char KeyNodekey[121];
    bf.read(KeyNodekey, newBucket + 12);
    
    pair<string, string> VandCPair;
    vector< pair<string,string> > ValueNodes;
    
    //store the value from the ValueNode
    char ValueNodeValue[121], ValueNodeValue1[121];
    int ValueNodeLocation;
    bf.read(ValueNodeLocation, newBucket);
    bf.read(ValueNodeValue, ValueNodeLocation + 4);
    strcpy(ValueNodeValue1, ValueNodeValue);
    
    //store the context from the ValueNode
    char ValueNodeContext[121], ValueNodeContext1[121];
    bf.read(ValueNodeContext, ValueNodeLocation + 125);
    strcpy(ValueNodeContext1, ValueNodeContext);
    
    //create the first pair in the vector
    VandCPair = make_pair(ValueNodeValue, ValueNodeContext);
    ValueNodes.push_back(VandCPair);
    
    ////////////////////////////////////
    
    //insert everything into the vector that'll be inserted into the iterator
    int nextPtr;
    bf.read(nextPtr, ValueNodeLocation); // nextPtr should be pointing to the second node in the list
    
    while (nextPtr != -1)
    {
        //store the value from the ValueNode
        bf.read(ValueNodeValue, nextPtr + 4);
        
        //store the context from the ValueNode
        bf.read(ValueNodeContext, nextPtr + 125);
        
        //create the first pair in the vector
        VandCPair = make_pair(ValueNodeValue, ValueNodeContext);
        ValueNodes.push_back(VandCPair);
        
        bf.read(nextPtr, nextPtr);
    }
    
    //WE'LL USE THE NEW ITERATOR THAT PASSES IN A VECTOR
    //create a new iterator and pass in those values as well as the location of the KeyNode
    Iterator finalIterator(ValueNodes, &bf, firstPastBucket, ValueNodeLocation, KeyNodekey);
    
    return finalIterator;
}

int DiskMultiMap::erase(const std::string& key, const std::string& value, const std::string& context)
{
    if (!bf.isOpen()) return 0;
    
    int totalErased = 0;
    int bucketlocation = hash(key);
    
    int bucketValue;
    bf.read(bucketValue, bucketlocation);
    if (bucketValue == -1) return 0; // key wasn't found
    
    // bucketValue = LOCATION OF KEYNODE
    
    int currentNode;
    bf.read(currentNode, bucketValue);
    
    // next = LOCATION OF FIRST VALUENODE
    int tracker = bucketValue; /// ADDRESS OF KEYNODE
    while (currentNode != -1)
    {
        char existingValue[121]; // for comparing the value
        char existingContext[121]; // for comparing the context
        
        bf.read(existingValue, currentNode+4);
        bf.read(existingContext, currentNode+125);
        
        //check if there's a complete match now
        if (existingValue == value && existingContext == context)
        {
            int byte4; // read the current value of the pointer at offset 4
            int valueLocation = currentNode; // copy the current position
            bf.read(currentNode,currentNode);
            
            //link the past node to the next node
            bf.write(currentNode, tracker);
            
            //relink the deletion nodes
            bf.read(byte4, 4);
            bf.write(valueLocation, 4);
            bf.write(byte4, valueLocation);
            
            totalErased++;
            continue;
        }
        tracker = currentNode; // now store next is pointing to the next pointer
        bf.read(currentNode, currentNode);
    }
    
    //if there's no more values attached to a keynode, delete the keynode as well and empty the bucket
    bf.read(currentNode, bucketValue);
    if (currentNode == -1)
    {
        
        // swap the keynode's next pointer and byte4
        int byte4;
        bf.read(byte4, 4);
        int temp = byte4;
        bf.write(bucketValue, 4); // writes the address of the keynode into byte 4 (frees up space)
        bf.write(temp, bucketValue);
    
        //empty the bucket
        bf.write(-1, bucketlocation);
    }
    return totalErased;
}

int DiskMultiMap::insertKey(const std::string &key)
{
    int newKeyLocation = findInsert();
    // initialize and write the new KeyNode
    KeyNode newKeyNode;
    strcpy(newKeyNode.key, key.c_str());
    bf.write(newKeyNode, newKeyLocation); // write the new KeyNode into the file
    
    return newKeyLocation;
}

int DiskMultiMap::insertValue(const std::string& value, const std::string& context)
{
    //initialize the ValueNode
    int newValueNodeLocation = findInsert();
    ValueNode newValueNode;
    strcpy(newValueNode.value, value.c_str());
    strcpy(newValueNode.context, context.c_str());
    
    //write the ValueNode
    bf.write(newValueNode, newValueNodeLocation);
    return newValueNodeLocation;
}

void DiskMultiMap::printAll()
{
    int bucketValue;
    for (int i = 8; i != 8 + (NUMBUCKETS * 4); i += 4)
    {
        bf.read(bucketValue, i);
        if (bucketValue == -1) continue;
        
        //we've found a filled bucket
        int next;
        bf.read(next, bucketValue); // next now points to the location of the first ValueNode
        
        char KeyName[121]; // stores the KeyNode
        bf.read(KeyName, bucketValue+12);
        while (next != -1)
        {
            
            char s[121]; // for storing the strings we access
            bf.read(s, next+4);
            
            cout << "Key: " << KeyName << ", ";
            cout << "Value: " << s << ", ";
            
            bf.read(s, next+125);
            cout << "Context: " << s << endl;
            bf.read(next, next);
        }
    }
}

/////////////// ITERATOR ////////////////////////

DiskMultiMap::Iterator& DiskMultiMap::Iterator::operator++()
{
    if (isValid())
        vectorLocation++;
    if(!isValid())
    {
        currentOffset = -1;
        m_tuple.key = m_tuple.value = m_tuple.context = "";
    }
    return *this;
}

MultiMapTuple DiskMultiMap::Iterator::operator*()
{
    if (isValid())
    {
        m_tuple.value = ValueNodes[vectorLocation].first;
        m_tuple.context = ValueNodes[vectorLocation].second;
        return m_tuple;
    }
    
    //iterator is not pointing to a valid location:
    MultiMapTuple returnThis;
    returnThis.key = returnThis.value = returnThis.context = "";
    return returnThis;
}

//DiskMultiMap