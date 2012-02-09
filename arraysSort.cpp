#include<vector>
#include<algorithm>
#include<iostream>

int main(void){
    std::vector<double> a = {10, 30, 20}, b = {2, 3, 1};
    for(auto& ab:{&a,&b}) {
        // std::cout << typeid(ab).name() << std::endl;
        std::sort(begin(*ab), end(*ab)); // error 
    }
    for(auto& ab:{a,b}) {
        for (auto e : ab)
            std::cout << e << std::endl;
        std::cout << std::endl;
    }
}
