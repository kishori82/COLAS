#include "rlnc_rs.h"




int main() {

    char a[] = " Tip: When you define a named closure, the compiler generates a corresponding function class for it. Every time you call the lambda through its named variable, the compiler instantiates a closure object at the place of call. Therefore, named closures are useful for reusable functionality (factorial, absolute value, etc.), whereas unnamed lambdas are more suitable for inline ad-hoc computations. Unquestionably, the rising popularity of functional programming will make lambdas widely-used in new C++ projects. It’s true that lambdas don’t offer anything you haven’t been able to do before with function objects. However, lambdas are more convenient than function objects because the tedium of writing boilerplate code for every function class (a constructor, data members and an overloaded operator() among the rest) is relegated to compiler. Additionally, lambdas tend to be more efficient because the compiler is able to optimize them more aggressively than it would a user-declared function or class. Finally, lambdas provide a higher level of security because they let you localize (or even hide) functionality from other clients and modules.";

   
    printf("CHECKING REED-SOLOMON CODE  \n");
    ENCODED_DATA encoded_data_info = encode(15, 10, 30, a, strlen(a), reed_solomon) ;
    char *decoded = (char *)decode(15, 10, 30, encoded_data_info, reed_solomon);
    printf("DECODED DATA : %s\n",decoded);

    printf("\n");

    printf("CHECKING REED-SOLOMON CODE \n");
    encoded_data_info = encode(15, 10, 30, a, strlen(a), full_vector) ;
    decoded = (char *)decode(15, 10, 30, encoded_data_info, full_vector);
    printf("DECODED DATA : %s\n",decoded);



    return 0;

}
