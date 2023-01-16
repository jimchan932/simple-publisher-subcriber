#include <fstream>
#include <iostream>
#include <string>

using namespace std;

string leer_parrafo(istream &in)
{
  string result, line;

  // read and concatenate lines until two newlines are read
  while(getline(in, line)) 
    if(line.empty()) break;
    else result += line + ' ';

  // get rid of that last space
  result.erase(result.length() - 1);

  return result;
}

int main()
{
  cout << "Enter file name: ";
  std::string filename;
  std::cin >> filename;
  std::string javaBookReviewFilePath("../javaBookReview/");
  std::string pythonBookReviewFilePath("../pythonBookReview/");
  std::string cppBookReviewFilePath("../cppBookReview/");
  ifstream file(javaFilePath + filename);
  cout << "Reading paragraph from file...\n";
  string fichero_parrafo = leer_parrafo(file);
  file.close();

  cout << "The file's text is:\n"
       << fichero_parrafo
       << endl;
}
