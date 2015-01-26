/*
 * IW0HDV Extio
 *
 * Copyright 2015 by Andrea Montefusco IW0HDV
 * 
 * Licensed under GNU General Public License 3.0 or later. 
 * Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 *
 */

#if !defined __CONFIG_H__
#define      __CONFIG_H__

#include <tuple>
#include <utility> 
#include <iostream>
#include <fstream>
#include <sstream>

/**
 * for_each_in_tuple
 * generic template that applies a callable object over each element in a tuple
 *
 * as found in http://stackoverflow.com/questions/26902633/how-to-iterate-over-a-tuple-in-c-11
 */

template<size_t I = 0, typename Func, typename ...Ts>
typename std::enable_if<I == sizeof...(Ts)>::type
for_each_in_tuple(std::tuple<Ts...> &, Func func) {}

template<size_t I = 0, typename Func, typename ...Ts>
typename std::enable_if<I < sizeof...(Ts)>::type
for_each_in_tuple(std::tuple<Ts...> & tpl, Func func) 
{
    func ( std::get<I>(tpl) );
    for_each_in_tuple<I + 1>(tpl,func);
}
  
/**
 * file output functor
 */
struct functor_ostream
{
	functor_ostream (std::ofstream &str): myfile(str) {}
	std::ofstream &myfile;
	
    template<typename T>
    void operator () (T&& t)
    {
        myfile << t << std::endl;
    }
};

/**
 * file input functor
 */
struct functor_istream
{
	functor_istream (std::fstream &str): str_(str) {}
	std::fstream &str_;

	template < typename T >
    void operator () (T &&obj)
    {
		str_ >> obj;
    }
};
  
/**
 *  Config class
 *
 */ 
template < typename TUP >
class Config {
public:
    Config (const char *fn, TUP t) : t_(t)
    {
    	fn_ = fn;
    	restore ();
    }
    void save ()
    {
       	std::ofstream myfile;
        myfile.open (fn_.c_str());
       	functor_ostream fstr(myfile);
        for_each_in_tuple(t_,fstr);
       	myfile.close();
    }
    int restore ()
    {
       	std::fstream myfile;
       	myfile.open(fn_.c_str(), std::ios::in);
       	
       	if (!myfile.is_open()) return -1;
           functor_istream fstr(myfile);
           for_each_in_tuple(t_, fstr);
    	return 0;
    }
    ~Config () { save(); }

   template < int n , typename TI >
   TI get () { return  std::get<n>(t_) ; }

   template < int n , typename TI >
   void set (TI x) { std::get<n>(t_) = x; }

private:
   TUP t_;
   std::string fn_;
};


#endif