﻿#pragma once

#include "ybase.h"

//常量表达式优化
struct yconsteval
{
	static rbool op_const_exp(const tsh& sh,rbuf<tword>& v,rbool clear_sbk)
	{
		if(!tconf::c_op_const_eval)
		{
			return true;
		}
		for(int i=0;i<v.count();i++)
		{
			if(v[i].val!=rppoptr(c_sbk_l))
			{
				continue;
			}
			int right=ybase::find_symm_sbk(sh,v,i);
			if(right>=v.count())
			{
				continue;
			}
			int pos=i+1;
			if(pos+1>=right)
			{
				continue;
			}
			ifn(is_const_str(sh,v,pos,right))
			{
				continue;
			}
			int dst;
			ifn(eval(sh,
				ybase::vword_to_vstr(v.sub(pos,right)),dst))
			{
				rpperror(v[i],"const_eval");
				return false;
			}
			v[pos].val=rstr((uint)dst);
			ybase::clear_word_val(v,pos+1,right);
			if(clear_sbk)
			{
				v[i].val.clear();
				v[right].val.clear();
			}
		}
		ybase::arrange(v);
		return true;
	}

	static rbool eval(const tsh& sh,rbuf<rstr> src,int& dst,int level=0)
	{
		if(level>c_rpp_deep)
		{
			rpperror("const eval level overflow");
			return false;
		}
		level++;
		rbuf<rstr> soptr;
		rbuf<int> sopnd;
		soptr+=rppoptr(c_sharp_sharp);
		src+=rppoptr(c_sharp_sharp);
		for(int i=0;i<src.count();i++)
		{
			if(src[i]==rppoptr(c_sharp_sharp)&&
				soptr.get_top()==rppoptr(c_sharp_sharp))
			{
				break;
			}
			if(src[i].is_number())
			{
				int outopnd=src[i].toint();
				sopnd.push(outopnd);
			}
			elif(src[i]==rppoptr(c_sbk_l))
			{
				int right=ybase::find_symm_sbk(sh,src,i);
				if(right>=src.count())
				{
					rpperror("const eval miss )");
					return false;
				}
				int outopnd;
				ifn(eval(sh,src.sub(i+1,right),outopnd,level))
				{
					return false;
				}
				sopnd.push(outopnd);
				i=right;
			}
			elif(sh.m_optr.is_optr(src[i]))
			{
				if(soptr.empty())
				{
					rpperror("const eval miss soptr");
					return false;
				}
				rstr cur=src[i];
				if(!sh.m_optr.precede(soptr.top(),cur))
				{
					soptr.push(cur);
					continue;
				}
				rstr theta=soptr.pop();
				if(sopnd.empty())
				{
					rpperror("const eval miss sopnd");
					return false;
				}
				int second=sopnd.pop();
				if(sopnd.empty())
				{
					if(theta==rppoptr(c_plus))
					{
					}
					elif(theta==rppoptr(c_minus))
					{
						second=-second;
					}
					else
					{
						rpperror("const eval miss sopnd");
						return false;
					}
					sopnd.push(second);
					i--;
					continue;
				}
				int first=sopnd.pop();
				int outopnd;
				if(!calc(sh,first,second,theta,outopnd))
				{
					rpperror("const eval calc error");
					return false;
				}
				sopnd.push(outopnd);
				i--;
			}
		}
		if(sopnd.count()!=1)
		{
			return false;
		}
		dst=sopnd[0];
		return true;
	}

	static rbool calc(const tsh& sh,int first,int second,const rstr& theta,int& outopnd)
	{
		if(theta==rppoptr(c_plus))
		{
			outopnd=first+second;
		}
		elif(theta==rppoptr(c_minus))
		{
			outopnd=first-second;
		}
		elif(theta==rppoptr(c_mul))
		{
			outopnd=first*second;
		}
		elif(theta==rppoptr(c_divide))
		{
			if(second==0)
			{
				return false;
			}
			outopnd=first/second;
		}
		else
		{
			return false;
		}
		return true;
	}

	static rbool is_const_str(const tsh& sh,const rstr& s)
	{
		return (s.is_number()||s==rppoptr(c_sbk_l)||s==rppoptr(c_sbk_r)||
			s==rppoptr(c_plus)||s==rppoptr(c_minus)||s==rppoptr(c_mul)||
			s==rppoptr(c_divide));
	}

	static rbool is_const_str(const tsh& sh,const rbuf<tword>& v,int begin,int end)
	{
		for(int i=begin;i<end;i++)
		{
			if(!is_const_str(sh,v[i].val))
			{
				return false;
			}
		}
		return true;
	}
};
