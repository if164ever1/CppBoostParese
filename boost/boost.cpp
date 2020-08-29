// boost.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//
//BOOST_ALL_NO_LIB

#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <vector>



#include <boost/beast.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>


#pragma warning(disable : 4996)
//дві статичні змінні
const static std::string MAIN_API = "ip-api.com";
const static std::string API_ARGUMENTS = "/json/";

using namespace std;
namespace http = boost::beast::http;

namespace BTN = boost::property_tree;

class Client
{
public:
    static std::string getResponse(std::string ip) {    //статичний метод
        
        std::string argument = API_ARGUMENTS + ip;


        boost::asio::io_context io;         //інпут аутпут контексти
        boost::asio::ip::tcp::resolver resolver(io);        //резолвер
        boost::asio::ip::tcp::socket socket(io);    //IP сокер який викоритосвується для зєднання

        boost::asio::connect(socket, resolver.resolve(MAIN_API, "80")); //встановлюємо зєднання(80 - стандартний порт)

        http::request<http::string_body> req(http::verb::get, argument, 11);        //get запрос


        //http заголовки
        req.set(http::field::host, MAIN_API);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        //робимо відправку нашого запросу
        http::write(socket, req);

        //змінна для отримання нашого запросу
        std::string response; {
            boost::beast::flat_buffer buffer;   //буфер
            http::response<http::dynamic_body> res; //наш запрос
            http::read(socket, buffer, res);    //проводимо зчитування запросу
            response = boost::beast::buffers_to_string(res.body().data()); // в змінну записуємо те що отримали від запросу
        }
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);   //закриваємо зєднання
        return response;
    }


    //метод для отримання 
    std::string getFieldFromJson(std::string json, std::string field) {
        std::stringstream jsonEncode(json);
        boost::property_tree::ptree root;
        boost::property_tree::read_json(jsonEncode, root);

        if (root.empty())
        {
            return "";
        }
        return root.get<std::string>(field);
    }
};

class ParseJSON:public Client {
public:
    std::string ip, country, region, city,
        zip, orgInternet, asName;
      
    bool setIP() {
        ip = "query:\t" + getFieldFromJson(getResponse(""), "query");
        return true;
    }
    bool setCountry() {
        country = "continent:\t" + getFieldFromJson(getResponse(""), "country");
        return true;
    }
    bool setRegion() {
        region = "region:\t" + getFieldFromJson(getResponse(""), "region");
        return true;
    }
    bool setCity() {
        city = "city:\t\t" + getFieldFromJson(getResponse(""), "city");
        return true;
    }
    bool setZip() {
        zip = "zip:\t\t" + getFieldFromJson(getResponse(""), "zip");
        return true;
    }
    bool setOrgInternet() {
        orgInternet = "orgInternet:" + getFieldFromJson(getResponse(""), "org");
        return true;
    }
    bool setAsName() {
        asName = "as:\t\t" + getFieldFromJson(getResponse(""), "as");
        return true;
    }

    void showIP() { std::cout << ip << "\n"
                                << country << "\n"
                                << region << "\n"
                                 << city << "\n"
                                 << zip << "\n"
                                << orgInternet << "\n"
                                 << asName << "\n";
    }

    friend std::ofstream& operator<< (std::ofstream& ofs, ParseJSON& obj);
};

std::ofstream& operator<< (std::ofstream& ofs, ParseJSON& obj) {
    
    ofs << obj.ip << "\n" << 
            obj.country << "\n" << obj.region << "\n" << 
                obj.city << "\n" << obj.zip << "\n" << 
                    obj.orgInternet << "\n" << obj.asName << "\n";
    return ofs;
}

void setterMethodParseJSON(ParseJSON& p) {
    p.setIP();
    p.setCountry();
    p.setRegion();
    p.setCity();
    p.setZip();
    p.setOrgInternet();
    p.setAsName();

}

class SaveInformation :public ParseJSON {
    ofstream file_info;
    std::string fileNameDIR;
public:
    SaveInformation(std::string object) {
        fileNameDIR = "json1.json";
        createFile(object);
    }

    bool createFile(std::string object) {
        file_info.open(fileNameDIR);
        if (!file_info.is_open())
        {
            std::cout << "File not created!\n";
        }
        else {
            file_info << object;
        }
        file_info.close();
        return true;
    }
};


class JSParse {
    std::string str;
    int len;
    vector<std::string> getSections();  //функція для парсингу одного блоку строки(до коми) - push to std::vector
protected:
    map<std::string, std::string> data;
public:
    JSParse(std::string s);
    vector<std::string> getVector() { return getSections(); }
    void getKey_Value();  
    bool pushMap(std::string s1, std::string s2);
    void printData();
};

std::ostream& operator << (std::ostream& os, vector<std::string> obj) {
    for (size_t i = 0; i < obj.size(); i++)
    {
        os << obj[i] << "\n";
    }
    return os;
}

int main()
{
    Client client;
    ParseJSON parseJSON;
    
    std::string res = client.getResponse("");
    JSParse js(res);
    js.getKey_Value();
    js.printData();
    SaveInformation fileSave(res);  
    return 0;
}

vector<std::string> JSParse::getSections()
{
    std::string section;
    char symbol = ',';
    vector<std::string> sectionString;

    for (size_t i = 0; i < len; i++)
    {
        if (str[i] == symbol) {
            sectionString.push_back(section);
            section.clear();
            continue;
        }

        if (str[i] == '{' || str[i] == '}') continue;

        else
            section.push_back(str[i]);
    }
    return sectionString;
}

JSParse::JSParse(std::string s)
{
    str = s;
    len = str.length();
}

void JSParse::getKey_Value()
{
    std::string key;
    std::string value;
    char symbol = ',';
    char symbol2 = ':';

    for (size_t i = 0; i < len; i++)
    {
        if (str[i] == '{' || str[i] == '}') continue;
        while (str[i]!=symbol2)
        {
            key.push_back(str[i]);
            i++;
        }
        
        while (str[i] != symbol) {
            if (str[i] == '}')break;
            value.push_back(str[i]);
            i++;
        }

        pushMap(key, value);
        key.clear();
        value.clear();
    }
}

bool JSParse::pushMap(std::string s1, std::string s2)
{
    data.insert(make_pair(s1,s2));
    return true;
}

void JSParse::printData()
{
    for (auto i = data.begin(); i != data.end(); ++i)
    {
        std::cout << i->first << i->second << "\n";
    }
}