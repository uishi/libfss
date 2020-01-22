#include <chrono>
#include "fss-common.h"
#include "fss-server.h"
#include "fss-client.h"

const uint32_t kNumParty2 = 2;
const uint32_t kNumParty3 = 3;

int main()
{
    // Set up variables
    Fss fClient;

    // Initialize client, use 10 bits in domain as example
		uint32_t domain_bit = 10;
    initializeClient(&fClient, domain_bit, kNumParty2);
    
    // Equality FSS test
		// Eval(i, k0) + Eval(i, k1) = b iff i = a and 0 otherwise
    uint64_t a = 3;
    uint64_t b = 2;
    ServerKeyEq k0;
    ServerKeyEq k1;
    generateTreeEq(&fClient, &k0, &k1, a, b);
    
    // Initialize server
		Fss fServer;
    initializeServer(&fServer, &fClient);
    mpz_class ans0, ans1, fin;
    
		// Eval (DPF)
    ans0 = evaluateEq(&fServer, &k0, a);
    ans1 = evaluateEq(&fServer, &k1, a);
    fin = ans0 - ans1;
    cout << "FSS Eq Match (should be non-zero): " << fin << endl; // equal to b
    
		// Eval (DPF)
    ans0 = evaluateEq(&fServer, &k0, (a-1));
    ans1 = evaluateEq(&fServer, &k1, (a-1));
    fin = ans0 - ans1;
    cout << "FSS Eq No Match (should be 0): " << fin << endl;

    // Less than FSS test
    ServerKeyLt lt_k0;
    ServerKeyLt lt_k1;
    
    initializeClient(&fClient, domain_bit, kNumParty2);
    generateTreeLt(&fClient, &lt_k0, &lt_k1, a, b);
    initializeServer(&fServer, &fClient);

    // Eval (Interval)
    uint64_t lt_ans0, lt_ans1, lt_fin;
    lt_ans0 = evaluateLt(&fServer, &lt_k0, (a-1));
    lt_ans1 = evaluateLt(&fServer, &lt_k1, (a-1));
    lt_fin = lt_ans0 - lt_ans1;
    cout << "FSS Lt Match (should be non-zero): " << lt_fin << endl;

    // Eval (Interval)
    lt_ans0 = evaluateLt(&fServer, &lt_k0, a);
    lt_ans1 = evaluateLt(&fServer, &lt_k1, a);
    lt_fin = lt_ans0 - lt_ans1;
    cout << "FSS Lt No Match (should be 0): " << lt_fin << endl;

    // Equality FSS test for multi-parties
    MPKey mp_keys[3];
    initializeClient(&fClient, domain_bit, kNumParty3);
    generateTreeEqMParty(&fClient, a, b, mp_keys);

    initializeServer(&fServer, &fClient);
    uint32_t mp_ans0 = evaluateEqMParty(&fServer, &mp_keys[0], a);
    uint32_t mp_ans1 = evaluateEqMParty(&fServer, &mp_keys[1], a);
    uint32_t mp_ans2 = evaluateEqMParty(&fServer, &mp_keys[2], a);
    uint32_t xor_mp = mp_ans0 ^ mp_ans1 ^ mp_ans2;
    cout << "FSS Eq Multi-Party Match (should be non-zero): " << xor_mp << endl;

    mp_ans0 = evaluateEqMParty(&fServer, &mp_keys[0], (a+1));
    mp_ans1 = evaluateEqMParty(&fServer, &mp_keys[1], (a+1));
    mp_ans2 = evaluateEqMParty(&fServer, &mp_keys[2], (a+1));
    xor_mp = mp_ans0 ^ mp_ans1 ^ mp_ans2;
    cout << "FSS Eq Multi-Party No Match (should be 0): " << xor_mp << endl;

    size_t rounds = 100000;
    auto t_begin = std::chrono::high_resolution_clock::now();
    for(size_t i=0; i<rounds; i++) {
        volatile auto x = evaluateEq(&fServer, &k0, i);
    }
    for(size_t i=0; i<rounds; i++) {
        volatile auto x = evaluateLt(&fServer, &lt_k0, i);
    }
    for(size_t i=0; i<rounds; i++) {
        volatile auto x = evaluateEqMParty(&fServer, &mp_keys[1], a);
    }
    auto t_end = std::chrono::high_resolution_clock::now();
    std::cout << "Benchmark result: " <<
     std::chrono::duration<double, std::milli>(t_end - t_begin).count()
     << " ms" << endl;
    return 0;
}
