#include <zhele/flash.h>

int main()
{
    char data[] = "Some data for store into flash";
    Zhele::Flash::ErasePage(10);
    auto dataStoreAddress = reinterpret_cast<void*>(Zhele::Flash::PageAddress(10));
    Zhele::Flash::WriteFlash(dataStoreAddress, data, sizeof(data));

    for (;;)
    {
    }
}
