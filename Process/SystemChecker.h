#include <fstream>
#include <string>

class MemoryChecker
{
  public:
    MemoryChecker()
    {
        pid = getpid();
        checkFileName += ("/proc/" + std::string(std::to_string(pid)) + "/status");
        std::cout<<checkFileName<<std::endl;
        checkFile = new fstream();
    }

    ~MemoryChecker()
    {
        if(checkFile->is_open())
        {
            checkFile->close();
        }
        delete checkFile;
    }
    void checkMemory()
    {
       /* if (!checkFile->is_open())
        {
            checkFile->open(checkFileName, std::ios::in);
        }
        assert(checkFile);
        int line = 0;
        std::string memoryUsed;
        while (std::getline(*checkFile, memoryUsed))
        {
            line++;
            if (line == vmRSSLine)
            {
                MemoryUsed = getMemoryToInt(memoryUsed);
                break;
            }
        }*/
        //std::cout<<"check memory: "<<MemoryUsed<<std::endl;
    }

    unsigned int getMemoryToInt(std::string memoryUsed)
    {
        std::string::iterator first =
            std::find(memoryUsed.begin(), memoryUsed.end(), ':');
        std::string key(memoryUsed.begin(), first);
        if (key == "VmRSS")
        {
            std::string::reverse_iterator last1 =
                std::find(memoryUsed.rbegin(), memoryUsed.rend(), ' ');
            std::string::reverse_iterator last2 =
                std::find(last1 + 1, memoryUsed.rend(), ' ');
            return std::atoi(std::string(&*last2, &*last1).c_str());
        }
        else
        {
            abort();
        }
    }

  private:
    int pid;
    std::string checkFileName;
    std::fstream *checkFile;
    int MemoryUsed;
    const int vmRSSLine = 21;
};