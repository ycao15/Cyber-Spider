//
//  main.cpp
//  Project 4 - Cyber Spider
//
//  Created by Yuching Cao on 3/5/16.
//  Copyright © 2016 Yuching Cao. All rights reserved.
//

#include <iostream>
#include "DiskMultiMap.h"
#include "IntelWeb.h"
using namespace std;
/*
int main()
{
    DiskMultiMap m;
    m.createNew("hello.dat", 5);
    
    m.insert("HELLOWORLD", "NACHENBERG", "SMALLBERG");
    m.insert("HELLOWORLD", "BAD.EXE", "MMMMMMM");
    m.insert("HELLOWORLD", "BAD.EXE", "MMMMMMM");
    m.insert("HELLOWORLD", "GOOGLE.COM", "REINMANCS33");
    m.insert("HELLOWORLD", "GOOGLE.COM", "REINMANCS33");
    m.insert("TEST123", "WHATVALUEISTHIS", "THISCONTEXT");
    m.insert("TEST123", "WHATVALUEISTHIS", "THISCONTEXT");
    
    cout << m.erase("TEST123", "WHATVALUEISTHIS", "THISCONTEXT") << endl;
    
    //m.printAll();

    DiskMultiMap::Iterator iter = m.search("HELLOWORLD");
    while (iter.isValid())
    {
        MultiMapTuple mmt = *iter;
        cout << "Key: " + mmt.key + " Value: " + mmt.value + " Context: " + mmt.context << endl;
        ++iter;
    }
}
*/

int main() {
    
    IntelWeb iw;
    iw.createNew("test", 100);
    
    string s = (string)"m26500 AA 111\n" +
    "m491 sss zzz\n" +
    "m491 zzz 222\n" +
    "m491 zzz xxx\n" +
    "m491 sss 333\n" +
    "m7711 CC 444\n" +
    "m9040 DD yyy\n" +
    "m15006 EE kkk\n" +
    "m15006 kkk sss\n" +
    "m15006 sss 555\n" +
    "m15006 sss AA\n" +
    "m15006 555 666\n" +
    "m19072 FF sss\n" +
    "m3430 GG eee\n" +
    "m3430 eee vvv\n" +
    "mloop 112 223\n" +       //these logs creat a loop to check your crawl() for looping
    "mloop 223 331\n" +       //
    "mloop 331 112\n" +       //
    "mlp 112 AA\n" +
    "m00 AA 111";      //if this line isn't included in your map you PROBABLY HAVE A PROBLEM WITH ingest()!!
    
    
    iw.ingest("testData.dat");
    
    //will test ingest
    //iw.printAllContents();          //UNCOMMENT THIS LINE
    //cout << endl << endl << endl;
    
    //use the below code to test openExisting
    
    //cout << "open again-----------------------------------" << endl;
     iw.openExisting("test");
     //iw.printAllContents();
     //cout << endl << endl << endl;
     IntelWeb iw2;
     //cout << "open again from new IntelWeb-----------------" << endl;
     iw2.openExisting("test");
     //iw2.printAllContents();
     //cout << endl << endl << endl;
    
     
    /*use the below code to test crawl
    vector<string> indicators = {"AA"};
     unsigned int minPrevalenceToBeGood = 4;
     vector<string> badEntitiesFound;
     vector<InteractionTuple> badInteractions;
     
     cout << "There were " <<
     iw.crawl(indicators, minPrevalenceToBeGood, badEntitiesFound, badInteractions)
     << " bad entities found:" << endl;
     
     for (int i = 0; i < badEntitiesFound.size(); i++) {
     cout << badEntitiesFound[i] << endl;
     }
     cout << endl << "Here's the evidence:" << endl;
     for (int i = 0; i < badInteractions.size(); i++) {
     InteractionTuple tuple = badInteractions[i];
     cout << "from: " << tuple.from << "  to: " << tuple.to << "  computer: " << tuple.context << endl;
     }
    */
     
    //use the below code to test purge
     iw.printAllContents();
     cout << endl << endl << endl;
     
     iw.purge("sss");
    
    cout << "-----------------" << endl;
     iw.printAllContents();
     cout << endl << endl << endl;
     
     iw.purge("zzz");
    
    cout << "---------------" << endl;
     iw.printAllContents();
    
    return 1;
}
