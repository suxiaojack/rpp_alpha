#pragma once

#ifndef _RPP
#include "../xlib/xsock.h"
#endif
#include "../front/yclass.h"
#include "zinterpret.h"
#include "zjit.h"
#include "zgpp.h"

//main函数进行参数解析
struct zmain
{
	static int rpp_main(uchar* cont)
	{
		tsh sh;
		sh.m_main_cont=cont;
		ybase::init_path(sh.m_path);
		ybase::init_match(sh.m_match);
#ifndef _RPP
		xsock_t sock_t;
		zjitf::get_psh()=&sh;
#endif
		rbuf<rstr> vparam=rf::get_param();
		rstr name;
		if(vparam.count()>=3)
		{
			name=vparam[2];
			if(vparam[1]==rstr("-jit"))
			{
				sh.m_mode=tsh::c_jit;
			}
			elif(vparam[1]==rstr("-interpret"))
			{
				sh.m_mode=tsh::c_interpret;
			}
			elif(vparam[1]==rstr("-win"))
			{
				sh.m_mode=tsh::c_win;
			}
			elif(vparam[1]==rstr("-grub"))
			{
				sh.m_mode=tsh::c_grub;
			}
			elif(vparam[1]==rstr("-gpp"))
			{
				sh.m_mode=tsh::c_gpp;
			}
			elif(vparam[1]==rstr("-gpp64"))
			{
				sh.m_mode=tsh::c_gpp;
				sh.m_point_size=8;
			}
			else
			{
				name=vparam[1];
				sh.m_mode=tsh::c_jit;
			}
		}
		elif(vparam.count()==2)
		{
			name=vparam[1];
			sh.m_mode=tsh::c_jit;
		}
#ifdef _MSC_VER
		name=rcode::gbk_to_utf8(name);
#endif
		rstrw wname=name;
		if(wname.get_left()==r_char('\"'))
		{
			wname=ybase::del_quote(wname.torstr());
		}
		if(wname.empty())
		{
			rpperror("miss file name");
			return false;
		}
		wname=rdir::dir_std_w(wname);
		wname=ypre::get_abs_path(wname);
		sh.m_main_file=wname;
		sh.m_ret=0;
		ifn(compile(sh))
		{
			sh.m_ret=1;
		}
		return sh.m_ret;
	}

	static rbool load(tsh& sh)
	{
		ifn(ypre::process(sh))
		{
			rpperror("pre process error");
			return false;
		}
		ifn(yclass::process(sh))
		{
			rpperror("extract class error");
			return false;
		}
		ifn(yclasstl::process(sh))
		{
			rpperror("class template replace error");
			return false;
		}
		ifn(ymemb::process(sh))
		{
			rpperror("ymemb error");
			return false;
		}
		return true;
	}

	static rbool compile(tsh& sh)
	{
		ifn(load(sh))
		{
			return false;
		}
		if(sh.m_mode==tsh::c_jit)
		{
			ifn(zjit::run(sh))
			{
				return false;
			}
		}
		elif(sh.m_mode==tsh::c_interpret)
		{
			ifn(zinterpret::run(sh))
			{
				return false;
			}
		}
		elif(sh.m_mode==tsh::c_win)
		{
			if(!znasm::proc(sh))
			{
				return false;
			}
		}
		elif(sh.m_mode==tsh::c_grub)
		{
			if(!znasm::proc_grub(sh))
			{
				return false;
			}
		}
		elif(sh.m_mode==tsh::c_gpp)
		{
			if(!zgpp::process(sh))
			{
				return false;
			}
		}
		return true;
	}
};

//todo C++不支持mixin，只能这么写
rbool r_func_to_x86(tsh& sh,tfunc& tfi,tenv env)
{
	return zjit::func_to_x86(sh,tfi,env);
}
