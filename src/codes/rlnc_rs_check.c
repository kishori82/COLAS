#include "rlnc_rs.h"




int main() {

    char a[] = " Tip: When you define a named closure, the compiler generates a corresponding function class for it. Every time you call the lambda through its named variable, the compiler instantiates a closure object at the place of call. Therefore, named closures are useful for reusable functionality (factorial, absolute value, etc.), whereas unnamed lambdas are more suitable for inline ad-hoc computations. Unquestionably, the rising popularity of functional programming will make lambdas widely-used in new C++ projects. It’s true that lambdas don’t offer anything you haven’t been able to do before with function objects. However, lambdas are more convenient than function objects because the tedium of writing boilerplate code for every function class (a constructor, data members and an overloaded operator() among the rest) is relegated to compiler. Additionally, lambdas tend to be more efficient because the compiler is able to optimize them more aggressively than it would a user-declared function or class. Finally, lambdas provide a higher level of security because they let you localize (or even hide) functionality from other clients and modules.  ver the years, a number of tools for analyzing and understanding systems described using CSP have been produced. Early tool implementations used a variety of machine-readable syntaxes for CSP, making input files written for different tools incompatible. However, most CSP tools have now standardized on the machine-readable dialect of CSP devised by Bryan Scattergood, sometimes referred to as CSPM.[16] The CSPM dialect of CSP possesses a formally defined operational semantics, which includes an embedded functional programming language.  The most well-known CSP tool is probably Failures/Divergence Refinement 2 (FDR2), which is a commercial product developed by Formal Systems (Europe) Ltd. FDR2 is often described as a model checker, but is technically a refinement checker, in that it converts two CSP process expressions into Labelled Transition Systems (LTSs), and then determines whether one of the processes is a refinement of the other within some specified semantic model (traces, failures, or failures/divergence).[17] FDR2 applies various state-space compression algorithms to the process LTSs in order to reduce the size of the state-space that must be explored during a refinement check. FDR2 has been succeeded by FDR3, a completely re-written version incorporating amongst other things parallel execution and an integrated type checker. It is released by the University of Oxford, which also released FDR2 in the period 2008-12.[18] The Adelaide Refinement Checker (ARC) [19] is a CSP refinement checker developed by the Formal Modelling and Verification Group at The University of Adelaide. ARC differs from FDR2 in that it internally represents CSP processes as Ordered Binary Decision Diagrams (OBDDs), which alleviates the state explosion problem of explicit LTS representations without requiring theuse";
   
    int K = 40; //K
    int N = 55;
    int symbol_size = 70;

    printf("CHECKING REED-SOLOMON CODE  \n");
    ENCODED_DATA encoded_data_info = encode(N, K, symbol_size, a, strlen(a), reed_solomon) ;
    char *decoded = (char *)decode(N, K, symbol_size, encoded_data_info, reed_solomon);
    printf("DECODED DATA : %s\n",decoded);

    printf("\n");

    printf("CHECKING REED-SOLOMON CODE \n");
    encoded_data_info = encode(N, K, symbol_size, a, strlen(a), full_vector) ;
    decoded = (char *)decode(N, K, symbol_size, encoded_data_info, full_vector);
    printf("DECODED DATA : %s\n",decoded);



    return 0;

}
