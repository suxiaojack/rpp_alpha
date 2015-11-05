#include "back/zmain.h"

#ifdef __GNUC__
extern "C" int js_main(char* s)
{
	return zmain::rpp_main((uchar*)s);
}
#else
int main()
{
	return zmain::rpp_main(null);
}
#endif
