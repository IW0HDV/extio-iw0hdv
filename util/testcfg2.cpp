#include <type_traits>
#include <tuple>
#include <cstddef>
#include <string>
#include <utility>
#include <iostream>
#include <fstream>
#include <sstream>
#include <typeinfo>  //for 'typeid' to work  


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


// http://stackoverflow.com/a/16397153/3101963

struct functor_stdout
{
    template<typename T>
    void operator () (T&& t)
    {
        std::cout << t << std::endl;
    }
};

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

struct functor_istream
{
	functor_istream (std::fstream &str): str_(str) {}
	std::fstream &str_;

	#if 1
	template < typename T >
    void operator () (T &&obj)
    {
		str_ >> obj;
    }
	#else

    void operator () (int &obj)
    {
		str_ >> obj;
		std::cout << "->" << obj << std::endl;
    }
	
    void operator () (float &obj)
    {
		str_ >> obj;
		std::cout << "->" << obj << std::endl;
    }
	#endif
};


template <typename T>
void save_config (const char *fn, T tpl)
{
	std::ofstream myfile;
    myfile.open (fn);
	functor_ostream fstr(myfile);
    for_each_in_tuple(tpl,fstr);
	myfile.close();
}


template <typename T>
int read_config (const char *fn, T &tpl)
{
	std::fstream myfile;
	myfile.open(fn, std::ios::in);
	
	if (!myfile.is_open()) return -1;
    functor_istream fstr(myfile);
    for_each_in_tuple(tpl, fstr);
	
	return 0;
}

template<std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
  print(std::tuple<Tp...>& t)
  { }

template<std::size_t I = 0, typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), void>::type
  print(std::tuple<Tp...>& t)
  {
    std::cout << std::get<I>(t) << std::endl;
    print<I + 1, Tp...>(t);
  }

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


int main ()
{
	#if 0
    auto tpl = std::make_tuple(666, 2.1, 217, "urca");
	
	std::cout << "SAVE CONFIG:"  << std::endl;
    save_config("DELME2.txt", tpl);
	std::cout << "---------------"  << std::endl;

	print (tpl);
	
	
	std::tuple<int, float, int, char[10]> tr;
	
	if (read_config("DELME3.txt", tr) <0)
		std::cout << "Unable to open file"   << std::endl;
	else {
	    std::cout << "READ:"  << std::endl;
	    print (tr);
	}
    #endif
	typedef std::tuple<int, int, int, int, int, int, int> AIRSPY_CFG_T;

	Config<AIRSPY_CFG_T> as_cfg ("AIRSPY.txt", std::make_tuple(2500000, 5, 10, 6, 0, 0, 1));
	//as_cfg.restore ();
	
	std::cout << "AIR_SPY: 0: " << as_cfg.get<0,int>()  << std::endl;
	std::cout << "AIR_SPY: 1: " << as_cfg.get<1,int>()  << std::endl;
	std::cout << "AIR_SPY: 2: " << as_cfg.get<2,int>()  << std::endl;
	std::cout << "AIR_SPY: 3: " << as_cfg.get<3,int>()  << std::endl;
	std::cout << "AIR_SPY: 4: " << as_cfg.get<4,int>()  << std::endl;
	std::cout << "AIR_SPY: 5: " << as_cfg.get<5,int>()  << std::endl;
	std::cout << "AIR_SPY: 6: " << as_cfg.get<6,int>()  << std::endl;
	
	as_cfg.set<1,int>(as_cfg.get<1,int>() + 1);
	
	if (as_cfg.get<1,int>() >10)
		as_cfg.set<1,int>(as_cfg.get<1,int>() - 1);

	if (as_cfg.get<1,int>() < 0)
		as_cfg.set<1,int>(as_cfg.get<1,int>() + 1);
		
    return 0;
}

