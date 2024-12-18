
#ifndef LOVE_RESOURCE_LOCATOR_H
#define LOVE_RESOURCE_LOCATOR_H
struct ResourceLocator{
  //WIP hep böyle boş kalmayacak. network, packaged, local, in memory ve cached arasında ayırıcı olmasını bekliyorum
  const char* path;
};
#endif //LOVE_RESOURCE_LOCATOR_H
