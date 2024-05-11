#include <memory>

#include "Application.h"

int main()
{
	std::unique_ptr<Application> app = std::make_unique<Application>();
	app->run();
	return 0;
}



// class Base{

// 	static const int value;
// };

// struct HolderInterface{

//     virtual ~HolderInterface() = default;
// };

// template<class T>
// struct DerivedHolder : HolderInterface{
// 	std::vector<T> data;


// };

// struct Derived1 : Base
// {

// };

// struct Derived2 : Base {
// };

// template <typename T>
// struct TypeValueMap{
// 	static const int value;
// };

// template <typename T>
// const int TypeValueMap<T>::value = -1;

// template <>
// const int TypeValueMap<Derived1>::value = 0;

// template <>
// const int TypeValueMap<Derived2>::value = 1;


// struct DataHolder {

// 	std::unordered_map<int, std::unique_ptr<HolderInterface>> all_data;

// 	DataHolder()
// 	{
// 		initialize<Derived1, Derived2>();
// 	}

// 	template <class T>
// 	std::vector<T>& get(){
		
// 		return static_cast<DerivedHolder<T>&>(*all_data.at(TypeValueMap<T>::value)).data;
// 	} 


// 	void remove(int entity_ind){}

// 	// template <class T>
// 	// std::vector<T>& get(int i){
		
// 	// 	// auto wtf = mapName.at(i);
// 	// 	return static_cast<DerivedHolder<decltype(*wtf)>>(*all_data.at(i)).p_data;
// 	// } 



// private:
// 	template <typename ...Types>
// 	void initialize(){
// 		(initializeOne<Types>(), ...);
// 	}

// 	template <class T>
// 	void initializeOne (){
// 		all_data[TypeValueMap<T>::value] = std::make_unique<DerivedHolder<T>>();
// 	}
// };

// struct Updater
// {

// 	virtual void update() = 0;
// };

// struct Derived1Updater : Updater
// {
// 	DataHolder* p_dh;

// 	virtual void update() override
// 	{
// 		auto& data = p_dh->get<Derived1>();

// 		for(auto datum : data){
			
// 		}
// 	}
// };

