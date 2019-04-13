#include <fstream>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <list>
#include <sstream>
#include <boost/filesystem.hpp>
#include <cassert>
#include <chrono>
#include <algorithm>

class MusicAlbum;

class MusicTrack
{
    const unsigned int begin_time;
    const std::string title;
    unsigned int end_time;
    const MusicAlbum &patris;

public:
    MusicTrack(const MusicAlbum &pat, unsigned int b_time, const std::string &titel)
        : begin_time(b_time),
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
    void match_end_times();

public:
    MusicAlbum(const std::string &filename, const std::string &chart)
        : m_filename(filename),
          m_chart(chart)
    {
        if (!boost::filesystem::exists(boost::filesystem::path(m_filename)))
            throw std::runtime_error("Your album audio file seems not to exist!");
        parse_from_file();
        match_end_times();
    }

    void ffextract()
    {
        for (auto &x : track_list)
            x.ffextract();
    }
    const std::string &get_filename() const
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
        assert(ret.find('\\') == std::string::npos);
        assert(ret.find('/') == std::string::npos);
        if (ret.find_first_of('-') != std::string::npos)
            ret = ret.substr(0, ret.find_first_of('-'));
        if (ret.back() == ' ')
            ret.pop_back();
        return ret;
    }
};

void MusicAlbum::match_end_times()
{
    unsigned int lbegin_time = 0;
    for (auto i = track_list.rbegin(); i != track_list.rend(); i++)
    {
        i->set_end_time(lbegin_time);
        lbegin_time = i->get_begin_time();
    }
}

void MusicAlbum::parse_from_file()
{
    std::ifstream ifs;
    try
    {
        ifs.open(m_chart);
        if (!ifs.good() || !ifs.is_open())
            throw std::runtime_error("Unable to access the given chart file.");
    }
    catch (std::exception &e)
    {
        throw std::runtime_error("Chart opening error: " + std::string(e.what()));
    }
    while (!ifs.eof())
    {
        std::string buf;
        std::getline(ifs, buf);
        if (buf.size() > 2)
        {
            try
            {
                parse_line(buf);
            }
            catch (std::exception &e)
            {
                ifs.close();
                throw std::runtime_error("Chart parsing error: " + std::string(e.what()));
            }
        }
    }
    ifs.close();
}

void MusicAlbum::parse_line(std::string buf)
{
    std::cout << "[CHART PARSER] Processing line: \"" << buf << "\"\n";
    unsigned int hrs_time = 0;
    unsigned int min_time = 0;
    unsigned int sec_time = 0;
    std::stringstream dataflow;

    std::string::iterator iter = buf.begin();
    if (std::count_if(buf.begin(), buf.end(), [](char chu) { return chu == ':'; }) == 0 || std::count_if(buf.begin(), buf.end(), [](char chu) { return chu == ':'; }) > 2)
        std::runtime_error("The line \"" + buf + "\" has problems with the number of colons.");
    if (std::count_if(buf.begin(), buf.end(), [](char chu) { return chu == ':'; }) > 1)
    {
        for (; iter != buf.end() && *iter != ':'; iter++)
        {
            dataflow.put(*iter);
        }
        if (*iter == ':')
            iter++;
        dataflow >> hrs_time;
        dataflow.str("");
        dataflow.clear();
    }
    for (; iter != buf.end() && *iter != ':' && *iter != '-'; iter++)
    {
        dataflow.put(*iter);
    }
    if (*iter == ':')
        iter++;
    else
        throw std::runtime_error("Wrong input! [at minutes]");
    dataflow >> min_time;
    dataflow.str("");
    dataflow.clear();
    for (; iter != buf.end() && *iter != '-'; iter++)
    {
        dataflow.put(*iter);
    }
    if (*iter == '-')
        iter++;
    else
        throw std::runtime_error("Wrong input! [at seconds]");
    dataflow >> sec_time;

    while (*iter == ' ' && iter != buf.end())
        iter++;
    dataflow.clear();
    dataflow.str("");
    while (iter != buf.end())
    {
        dataflow.put(*iter);
        iter++;
    }
    MusicTrack mustrax(*this, sec_time + 60 * min_time + 3600 * hrs_time, dataflow.str());
    track_list.push_back(mustrax);
    std::cout << "[CHART PARSER] Successfully added track with title: \"" << dataflow.str() << "\" which begins at: " << sec_time + 60 * min_time + 3600 * hrs_time << " seconds.\n";
}

void MusicTrack::ffextract()
{
    std::string ffcommand = "ffmpeg -i ";
    ffcommand.append(std::string("\"" + patris.get_filename()) + "\"");
    ffcommand.append(" -c:a copy -ss ");
    ffcommand.append(std::to_string(begin_time));
    if (end_time != 0)
    {
        unsigned int dur_time = end_time - begin_time;
        ffcommand.append(" -t ");
        ffcommand.append(std::to_string(dur_time));
    }
    ffcommand.push_back(' ');
    ffcommand.append("\"" + patris.get_band_name() + " - " + title + patris.get_extension() + "\"");
    std::cout << "\n\n\n[TRACK EXTRACTOR] Invoking ffmpeg with the following command: " << ffcommand << "\n\n\n";
    system(ffcommand.c_str());
}

int main(int argc, char **argv)
{
    using hi_res_clock = std::chrono::high_resolution_clock;
    if (argc >= 3)
    {
        try
        {
            std::cout << "[MAIN] Parsing album: " << argv[1] << " with chart: " << argv[2] << ".\n";
            auto inittime = hi_res_clock::now();
            MusicAlbum mus(argv[1], argv[2]);
            auto endtime = hi_res_clock::now();
            std::cout << "[MAIN] Finished parsing album in: " << std::chrono::duration_cast<std::chrono::microseconds>(endtime - inittime).count() << " microseconds.\n";
            mus.ffextract();
        }
        catch (std::exception &e)
        {
            std::cerr << "ERROR: " << e.what() << std::endl;
            return 1;
        }
    }
    else
    {
        std::cout << "FFALBUM-SPLIT \n\n";
        std::cout << "This is a tool for splitting albums using ffmpeg. \nA whole album audio file can be split into single tracks from the album.\n(Ffmpeg required to be accessible by typing 'ffmpeg' in the system console.)\n";
        std::cout << "Usage: " << argv[0] << " <album audio file> <chart file>\n\n";
        std::cout << "album audio file - any audio file whose extension is supported by your version of ffmpeg\n";
        std::cout << "chart file - track chart for your album\n\n";
        std::cout << "The track chart needs to be written as in the following example:\n";
        std::cout << "h:m:s - <Track title>\nh:m:s - <Another title>\n\twhere 'h' stands for hours, 'm' stands for minutes and 's' stands for seconds.\n";
        std::cout << "The title of the track must not contain any quotation marks - \"   Colons will also cause errors - :\n";
        std::cout << "You can also alternatively omit hours and use the following format:\nm:s - <Track title>\n\n";
        std::cout << "It is required for the tracks in the chart to be listed in separate lines.\n\n";
        std::cout << "The album audio file is expected to be named in the following way: \n\n<Band name> - <Album title>\n\nThe individual titles will then be named accordingly to that in the following manner:\n\n";
        std::cout << "<Band name> - <Track title>\n"
                  << std::endl;
    }

    return 0;
}