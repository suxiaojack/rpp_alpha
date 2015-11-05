#pragma once

#include "../xlib/xf.h"

template<typename T>
T* r_new(int count=1)
{
	T* p=(T*)(xf::malloc(r_size(int)+count*r_size(T)));
	if(p==null)
	{
		xf::print("memory insufficient");
		xf::exit(1);
		return null;
	}
	*(int*)p=count;
	p=(T*)((uchar*)p+r_size(int));
	T* start=p;
	T* end=start+count;
	for(;p<end;p++)
	{
#ifndef _RPP
		new(p)T();//((T*)p)->T::T();
#else
		p->T();
#endif
	}
	return start;
}

template<typename T>
T* r_new_n(int count=1)
{
	T* p=(T*)(xf::malloc(r_size(int)+count*r_size(T)));
	if(p==null)
	{
		xf::print("memory insufficient");
		xf::exit(1);
		return null;
	}
	*(int*)p=count;
	return (T*)((uchar*)p+r_size(int));
}

template<typename T>
void r_delete(T* p)
{
	uchar* start=(uchar*)p-r_size(int);
	T* end=p+*(int*)start;
	for(;p<end;p++)
	{
#ifndef _RPP
		p->~T();
#else
		T.~T(*p);
#endif
	}
	xf::free(start);
}

template<typename T>
void r_delete_n(T* p)//����ģ�����û��
{
	xf::free((uchar*)p-r_size(int));
}

//ģ���ػ�����ͷ�ļ���ֻ�ܰ���һ��
//���ע�͵���4���ػ�int��char���������ͻ��ʼ��Ϊ0��Ч���Ե�
template<>
uchar* r_new<uchar>(int count)
{
	return r_new_n<uchar>(count);
}

template<>
void r_delete<uchar>(uchar* p)
{
	r_delete_n<uchar>(p);
}

template<>
ushort* r_new<ushort>(int count)
{
	return r_new_n<ushort>(count);
}

template<>
void r_delete<ushort>(ushort* p)
{
	r_delete_n<ushort>(p);
}
