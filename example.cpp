/*************************************************************************
	> File Name: prime_thread.cpp
	> Author: 
	> Mail: 
	> Created Time: Tue 10 Nov 2020 11:25:15 AM EST
 ************************************************************************/

#include <iostream>
#include <functional>
#include "ThreadPool.h"


int is_prime(int x) {
    if (x <= 1) return 0;
    for (int i=2; i*i <= x; i++) {
        if (x % i == 0) return 0;
    }
    return 1;
}

void prime_cnt(int &ans, int l, int r) {
    int cnt = 0;
    for (int i=l; i<=r; i++) {
        cnt += is_prime(i);
    }
    __sync_fetch_and_add(&ans, cnt);
    //ans += cnt;
    return;
}

int main() {
    int cnt = 0;
    vk::thread_pool tp(5);
    tp.start();
    for (int i=1; i<=10; i++) {
        tp.add_task(prime_cnt, std::ref(cnt), (i-1)*100000+1, i*100000);
    }
    tp.stop_until_empty();
    //tp.stop();
    std::cout << cnt << std::endl;
    return 0;
}

