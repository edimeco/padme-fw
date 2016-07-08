/**
 * \file TConfigParser.hh
 * \author Georgi Georgiev <ggeorge@cern.ch>
 * \date 2016-07-07
 */

#include<fstream>
#include<iostream>
#include<string>
#include<vector>
#include<sstream>
#include<map>

#ifndef _TConfigParser_hh_
#define _TConfigParser_hh_ 0

// to be included in makefile

class TVConfigParser{
  public:
    void Verbose(){
      std::cout<<std::endl;
      std::cout<<"Process config file "<<fFileName<<std::endl;
      for(std::map<std::string,std::map<std::string,std::vector<std::string> > > ::iterator
          groupmap_it=fConfigLines.begin(); groupmap_it!=fConfigLines.end();++groupmap_it){
        const std::string& grouptag=groupmap_it->first;
        for(std::map<std::string,std::vector<std::string> > ::iterator
            map_it=groupmap_it->second.begin(); map_it!=groupmap_it->second.end();++map_it){
          const std::string& field=map_it->first;
          for(std::vector<std::string>::iterator vec_it=map_it->second.begin();vec_it!=map_it->second.end();++vec_it){
            const std::string& value=*vec_it;
            std::cout<<grouptag<<" \""<<map_it->first<<"\"  <-  \""<<*vec_it<<"\""<<std::endl;
          }
        }
      }
      std::cout<<"Finish processing"<<std::endl;
    }

    TVConfigParser(std::string icfilename){
      fFileName=icfilename;
      std::ifstream icfile(icfilename,std::ifstream::in);
      int lineNum=0;
      if(icfile.is_open()){
        std::string cfggroupname="[Default]";
        while(!icfile.eof()){
          std::string tmpstr;
          std::getline(icfile,tmpstr);
          ++lineNum;
          //tmpstr.erase(std::remove(tmpstr.begin(),tmpstr.end(),' '),tmpstr.end());
          //std::cout<<"proc: "<<tmpstr<<std::endl;

          if(!tmpstr.length()>0){
            //std::cout<<"empty: "<<tmpstr<<std::endl;
            continue;}
          tmpstr=tmpstr.substr(
              tmpstr.find_first_not_of(" \t\f\v\n\r"),
              tmpstr.find_last_not_of(" \t\f\v\n\r")+1
              );

          if(tmpstr.find_first_of('#')!=std::string::npos)
            tmpstr.erase(tmpstr.find_first_of('#'),tmpstr.length()); // # comment char

          if(!tmpstr.length()>0){
            //std::cout<<"empty: "<<tmpstr<<std::endl;
            continue;}

          if(tmpstr[0]=='[') {
            if(tmpstr[tmpstr.length()-1]==']'){
              cfggroupname=tmpstr;
            }else{
              std::cerr<<"ERROR: Syntax error file \"" <<icfilename<<":"<<lineNum<<"\""<<std::endl;
              std::cerr<<"ERROR: Group: [GroupName]"<<std::endl;
              std::cerr<<std::endl;
            }
            continue;
          }

          std::stringstream ss(tmpstr);
          std::string configfield;
          if(ss>>configfield){
            //fConfigLines[configfield].clear();
          }
          while(ss>>tmpstr) {
            //std::cout<<"push_back: "<<tmpstr<<std::endl;
            fConfigLines[cfggroupname][configfield].push_back(tmpstr);
          }
          if(!fConfigLines[cfggroupname][configfield].size()>0){
            std::cerr<<"ERROR: Syntax error " <<icfilename<<":"<<lineNum<<std::endl;
            std::cerr<<"ERROR: Group: \""<<cfggroupname<<"\" Field: \"" <<configfield<<"\"" <<std::endl;
            std::cerr<<"ERROR: At least one argument expected"<<std::endl;
            std::cerr<<std::endl;
          }
        }
      }else{
        std::cerr<<"File not found: \""<<icfilename<<"\""<<std::endl;
        std::cerr<<std::endl;
      }
      Verbose();
    }
    std::string GetConfig(std::string grp,std::string cfg){
      if(fConfigLines.find(grp)!=fConfigLines.end()){
        std::map<std::string,std::vector<std::string> >& groupmap=fConfigLines[grp];
        if(groupmap.find(cfg)!=groupmap.end()){
          std::vector<std::string>& cmdvec=groupmap[cfg];
          if(cmdvec.size()==1){
            std::cout<< cmdvec[0]<<std::endl;
            return cmdvec[0];
          }
        }
      }
      std::cerr<<"Problem with group "<<grp<<"  field "<<cfg<<std::endl;
      std::cerr<<std::endl;
      return "";
    }
    bool HasConfig(std::string grp, std::string cfg){
      if(fConfigLines.find(grp)!=fConfigLines.end()){
        std::map<std::string,std::vector<std::string> >& groupmap=fConfigLines[grp];
        if(groupmap.find(cfg)!=groupmap.end()){
          std::vector<std::string>& cmdvec=groupmap[cfg];
          if(cmdvec.size()==1){
            return true;
          }
        }
      }
      return false;
    }

  protected:
    std::string fFileName;
    std::map<std::string,std::map<std::string,std::vector<std::string> > > fConfigLines;

};

class TDetVConfigParser:TVConfigParser{
  public:
    TDetVConfigParser(std::string a):TVConfigParser(a){ }
};


class TTargetConfigParser:TDetVConfigParser{
  public:
    TTargetConfigParser(std::string a):TDetVConfigParser(a){ }
};
class TEVetoConfigParser:TDetVConfigParser{
  public:
    TEVetoConfigParser(std::string a):TDetVConfigParser(a){ }
};
class TPVetoConfigParser:TDetVConfigParser{
  public:
    TPVetoConfigParser(std::string a):TDetVConfigParser(a){ }
};
class THEPVetoConfigParser:TDetVConfigParser{
  public:
    THEPVetoConfigParser(std::string a):TDetVConfigParser(a){ }
};
class TECalConfigParser:TDetVConfigParser{
  public:
    TECalConfigParser(std::string a):TDetVConfigParser(a){ }
};
class TSACConfigParser:TDetVConfigParser{
  public:
    TSACConfigParser(std::string a):TDetVConfigParser(a){ }
};


class TGlobalConfigParser:TVConfigParser{
  public:
    TGlobalConfigParser(std::string a):TVConfigParser(a){}
    std::string GetConfig(std::string grp,std::string cfg){return TVConfigParser::GetConfig(grp,cfg);}
    bool HasConfig(std::string grp,std::string cfg){return TVConfigParser::HasConfig(grp,cfg);}


};



#endif
