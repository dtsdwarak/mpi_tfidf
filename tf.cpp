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

vector<pair<string, int> > getKeywords(string filename) {

	string line,input_text="";
  ifstream myfile (filename);

	//Getting all words into input_text
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

	//Storing input_text into separate words in input_text_words
	vector<string> input_text_words;
  boost::split(input_text_words, input_text, boost::is_any_of(" "));


	map<string,int> found_keywords;
	unordered_map<string, int> stop_words = get_stop_words();
  int w_length,unique_keyword_count=0;

  for(string& word : input_text_words)
	{
   	//Remove extra spaces surrounding a word
	  boost::trim(word);
    w_length = word.length();

		//If the word is actually a word
    if(w_length)
    {
			//If the word is not a stop word then proceed
      if(stop_words[word] == 0)
			{
				//If not already present in keyword, Create the word
        if(!found_keywords[word])
				{
          found_keywords[word] = 1;
          unique_keyword_count++;
        }
				//If already present, increase the count
        else
				{
          found_keywords[word] += 1;
        }
      }
    }

  } //End of for loop

	vector<pair<string, int> > found_keywords_sort(found_keywords.begin(), found_keywords.end());

	struct
	{
    bool operator() (pair<string, int> a, pair<string, int> b)
    {
      return a.second > b.second;   // desc
    }
	} sort_keywords_desc;

  sort(found_keywords_sort.begin(), found_keywords_sort.end(), sort_keywords_desc);

	/*
	//Print Keywords
	for(int i=0;i<found_keywords_sort.size();i++){
  	cout<<found_keywords_sort[i].first<<" "<<found_keywords_sort[i].second<<endl;
  }
	*/

  return found_keywords_sort;

}
