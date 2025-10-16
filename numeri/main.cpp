#include <iostream>
#include <kernel.hpp>
int main() {
  std::cout << "Numeri Trading System Starting..." << std::endl;
  Kernel* kernel = new Kernel();
  kernel->start();

  while (true) {}
  return 0;
}