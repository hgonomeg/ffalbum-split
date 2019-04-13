#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <sstream>
#include <boost/filesystem.hpp>
#include <cassert>
#include <algorithm>

class MusicAlbum;

class MusicTrack
{
    const unsigned int begin_time;
    const std::string title;
    unsigned int end_time;
    const MusicAlbum& patris;
    public:

    MusicTrack(const MusicAlbum& pat, unsigned int b_time, const std::string& titel)
    :begin_time(b_time),
    patris(pat),
    title(titel)
    {
        
    }
    void set_end_time(unsigned int et)
    {
        end_time = et;
    }
    unsigned int get_begin_time()
    {
        return begin_time;
    }
    void ffextract();
};

class MusicAlbum
{
    std::list<MusicTrack> track_list;
    const std::string m_filename;
    const std::string m_chart;

    void parse_from_file();
    void parse_line(std::string buf);

    public:
    MusicAlbum(const std::string& filename,const std::string& chart)
    :m_filename(filename),
    m_chart(chart)
    {
    parse_from_file();
    match_end_times();
    }

    void ffextract()
    {
        for(auto& x: track_list) x.ffextract();
    }
    const std::string& get_filename() const
    {
        return m_filename;
    }

    std::string get_extension() const
    {
        using namespace boost::filesystem;
        std::string ret;
        path alpat(m_filename);
        ret = alpat.extension().generic_string();
        return ret;
    }

    std::string get_band_name() const
    {
        using namespace boost::filesystem;
        std::string ret;
        path alpat(m_filename);
        ret = alpat.stem().generic_string();
        assert(ret.find('\\')==std::string::npos);
        assert(ret.find('/')==std::string::npos);
        if(ret.find_first_of('-')!=std::string::npos) ret = ret.substr(0,ret.find_first_of('-'));
        return ret;
    }

    void match_end_times();
    
};

void MusicAlbum::match_end_times()
{
        unsigned int lbegin_time = 0;
        for(auto i=track_list.rbegin();i!=track_list.rend();i++)
        {
            i->set_end_time(lbegin_time);
            lbegin_time = i->get_begin_time();
        }
    }

void MusicAlbum::parse_from_file()
{
    std::ifstream ifs;
    ifs.open(m_chart);
    while(!ifs.eof())
    {
        std::string buf;
        std::getline(ifs,buf);
        if(buf.size()>2) parse_line(buf);
    }
    ifs.close();
}

void MusicAlbum::parse_line(std::string buf)
{
    
    unsigned int hrs_time = 0;
    unsigned int min_time = 0;
    unsigned int sec_time = 0;
    std::stringstream dataflow;

    std::string::iterator iter = buf.begin();
    if(std::count_if(buf.begin(),buf.end(),[](char chu){return chu==':';})>1)
    {
        for(;iter!=buf.end()&&*iter!=':';iter++)
        {
            dataflow.put(*iter);
        }
        if(*iter==':') iter++;
        dataflow>>hrs_time;
    }
    for(;iter!=buf.end()&&*iter!=':'&&*iter!='-';iter++)
    {
            dataflow.put(*iter);
    }
    if(*iter==':') iter++;
    else throw std::runtime_error("Wrong input!");
    dataflow>>min_time;
    for(;iter!=buf.end()&&*iter!='-';iter++)
    {
            dataflow.put(*iter);
    }
    if(*iter=='-') iter++;
    else throw std::runtime_error("Wrong input!");
    dataflow>>sec_time;
    while(*iter==' '&&iter!=buf.end()) iter++;
    dataflow.clear(); dataflow.str("");
    while(iter!=buf.end()) { dataflow.put(*iter); iter++; }
    MusicTrack mustrax(*this,sec_time+60*min_time+3600*hrs_time,dataflow.str());
    track_list.push_back(mustrax);
}


void MusicTrack::ffextract()
{
        std::string ffcommand = "ffmpeg -i ";
        ffcommand.append(std::string("\""+patris.get_filename())+"\"");
        ffcommand.append(" -c:a copy -ss ");
        ffcommand.append(std::to_string(begin_time));
        if(end_time!=0)
        {
            unsigned int dur_time = end_time-begin_time;
            ffcommand.append(" -t ");
            ffcommand.append(std::to_string(dur_time));
        }
        ffcommand.push_back(' ');
        ffcommand.append("\""+patris.get_band_name()+" - "+title+patris.get_extension()+"\"");
        system(ffcommand.c_str());
}

int main(int argc, char** argv)
{
    MusicAlbum mus(argv[1],argv[2]);
    mus.ffextract();
}