//g++ -std=c++11 -I /usr/lib/boost/include -L /usr/lib/boost/lib tfidf.cpp -lboost_regex
// If you've included boost libraries in LD_LIBRARY_PATH, just use $ g++ -std=c++11 tfidf.cpp -lboost_regex

#include <iostream>
#include <string>
#include <climits>
#include <map>
#include <unordered_map>
#include <fstream>
#include <algorithm>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include "stop_words.cpp"
using namespace std;

vector<pair<string, int> > getKeywords(string filename){

	string line,input_text="";
  	ifstream myfile (filename);
  	if (myfile.is_open())
  	{
    		while ( getline (myfile,line) )
    		{
			line = boost::regex_replace(line, boost::regex {"[^'a-zA-Z]"}, std::string {" "});
    			line = boost::regex_replace(line, boost::regex {"[ \r\n\t]+"}, std::string {" "});
    			transform(line.begin(), line.end(), line.begin(), ::tolower);
			input_text+=line+" ";
    		}
    		myfile.close();
  	}
	vector<string> input_text_words;
    boost::split(input_text_words, input_text, boost::is_any_of(" "));

	map<string,int> found_keywords;
	unordered_map<string, int> stop_words = get_stop_words();
    int w_length,unique_keyword_count;

    for(string& word : input_text_words)
    {
        boost::trim(word);

        w_length = word.length();

        if(w_length)
        {

            if(stop_words[word] == 0)
            {
                if(!found_keywords[word])
                {
                    found_keywords[word] = 1;
                    unique_keyword_count++;
                }
                else
                {

                        found_keywords[word] += 1;
                }

            }
        }
    }

	vector<pair<string, int> > found_keywords_sort(found_keywords.begin(), found_keywords.end());

    struct
    {
        bool operator() (pair<string, int> a, pair<string, int> b)
        {
            return a.second > b.second;   // desc
        }
    } sort_keywords_desc;

    sort(found_keywords_sort.begin(), found_keywords_sort.end(), sort_keywords_desc);

    for(int i=0;i<found_keywords_sort.size();i++){
        cout<<found_keywords_sort[i].first<<" "<<found_keywords_sort[i].second<<endl;
    }

    return found_keywords_sort;

}
