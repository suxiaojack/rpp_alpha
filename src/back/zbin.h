﻿#pragma once

#include "../front/ysent.h"
#include "../front/yword.h"
#include "zasm.h"

//生成二进制代码类
struct zbin
{
	//将一个函数翻译成二进制代码
	static rbool process(tsh& sh,tfunc& tfi)
	{
		if(!tfi.vasm.empty())
		{
			return true;
		}
		if(!cp_vword_to_vasm(sh,tfi,tenv()))
		{
			return false;
		}
		for(int i=0;i<tfi.vasm.count();i++)
		{
			if(!proc_asm(sh,tfi.vasm,tfi.vasm[i]))
			{
				rpperror(tfi.vasm[i],"asm error");
				return false;
			}
			tfi.vasm[i].ptfi=&tfi;
		}
		return true;
	}
	
	//翻译一条未解析的指令
	static rbool cp_call_asm(tsh& sh,tasm& item)
	{
		rmutex_t mutex_t(sh.m_mutex);
		if(item.ins.type!=0)
		{
			return true;
		}
		int i;
		for(i=1;i<item.vstr.count();i++)
		{
			if(item.vstr[i]==rppoptr(c_addr))
			{
				break;
			}
		}
		//一条指令里最多出现一次&,而且 &操作数 只能是最后一个操作数
		if(i>=item.vstr.count())
		{
			return false;
		}
		int addr_pos=i;
		i+=2;
		tclass* ptci=yfind::class_search(sh,item.vstr.get(i));
		if(ptci==null)
		{
			return false;
		}
		i+=2;
		rstr fname=item.vstr.get(i);
		tfunc* ptfi=yfind::func_search_dec(*ptci,fname);
		if(ptfi==null)
		{
			item.vstr.print();
			rf::printl();
			rpperror("can't find "+fname);
			return false;
		}
		ifn(process(sh,*ptfi))
		{
			return false;
		}
		item.vstr=item.vstr.sub(0,addr_pos-1);
		item.vstr+=rstr(r_to_uint(ptfi->vasm.begin()));
		if(!proc_asm(sh,item))
		{
			return false;
		}
		return true;
	}

	//从函数的词表编译到vasm
	static rbool cp_vword_to_vasm(tsh& sh,tfunc& tfi,tenv env)
	{
		ifn(ysent::process(sh,tfi,env))
		{
			rpperror(tfi,"sent error");
			return false;
		}
		ifn(zasm::process(sh,tfi))
		{
			rpperror(tfi,"asm error");
			return false;
		}
		return true;
	}

	static rbool proc_vasm(tsh& sh,rbuf<tasm>& vasm)
	{
		for(int i=0;i<vasm.count();i++)
		{
			if(!proc_asm(sh,vasm[i]))
			{
				return false;
			}
		}
		return true;
	}

	static rbool proc_asm(tsh& sh,tasm& oasm)
	{
		rbuf<tasm> vasm;
		return proc_asm(sh,vasm,oasm);
	}
	
	//将汇编代码翻译成二进制代码
	static rbool proc_asm(tsh& sh,rbuf<tasm>& vasm,tasm& oasm)
	{
		if(ybase::is_tag<rstr>(oasm.vstr))
		{
			oasm.ins.type=tins::c_nop_n;
			return true;
		}
		oasm.ins.type=sh.m_key.get_key_index(oasm.vstr.get_bottom());
		if(oasm.ins.type>tkey::c_rn)
		{
			rpperror(oasm);
			return false;
		}
		if(oasm.vstr.count()==2&&
			is_jmp_ins(oasm.ins.type)&&
			!sh.m_key.is_asm_reg(oasm.vstr[1]))
		{
			int line=oasm.vstr[1].toint();
			int i;
			for(i=0;i<vasm.count();i++)
			{
				if(ybase::is_tag<rstr>(vasm[i].vstr)&&
					vasm[i].vstr[0]==oasm.vstr[1])
				{
					break;
				}
			}
			if(i>=vasm.count())
			{
				rpperror(oasm);
				return false;
			}
			oasm.ins.type*=6;
			oasm.ins.first.type=topnd::c_imme;
			oasm.ins.first.val=(uint)(&vasm[i]);
			return true;
		}
		oasm.ins.type*=6;
		if(!a_asm(sh,oasm))
		{
			rpperror(oasm);
			return false;
		}
		return true;
	}

	static int find_comma(tsh& sh,rbuf<rstr>& v)
	{
		int count1=0;
		int count2=0;
		int i;
		for(i=1;i<v.count();i++)
		{
			if(rppoptr(c_sbk_l)==v[i])
			{
				count1++;
			}
			elif(rppoptr(c_sbk_r)==v[i])
			{
				count1--;
			}
			elif(rppoptr(c_mbk_l)==v[i])
			{
				count2++;
			}
			elif(rppoptr(c_mbk_r)==v[i])
			{
				count2--;
			}
			elif(count1==0&&count2==0&&v[i]==rppoptr(c_comma))
			{
				break;
			}
		}
		return i;
	}

	static rbool a_asm(tsh& sh,tasm& item)
	{
		int i=find_comma(sh,item.vstr);
		if(!a_opnd(sh,item,i-1,item.vstr.sub(1,i),item.ins.first))
		{
			return false;
		}
		if(item.ins.empty())
		{
			return true;
		}
		if(!a_opnd(sh,item,i+1,item.vstr.sub(i+1),item.ins.second))
		{
			return false;
		}
		if(item.ins.empty())
		{
			return true;
		}
		if(!obtain_qrun_type(item.ins))
		{
			return false;
		}
		if(item.ins.type==tins::c_calle_i)
		{
			ifn(sh.m_func_list.exist(r_to_pchar(item.ins.first.val)))
			{
				rpperror(r_to_pchar(item.ins.first.val));
				return false;
			}
			item.ins.first.val=r_to_int(sh.m_func_list[r_to_pchar(item.ins.first.val)]);
		}
		return true;
	}

	static rbool obtain_qrun_type(tins& ins)
	{
		if(ins.second.type==topnd::c_null)
		{
			if(ins.first.type==topnd::c_null)
			{
				;
			}
			elif(ins.first.type==topnd::c_imme)
			{
				;
			}
			elif(ins.first.type==topnd::c_reg)
			{
				ins.type+=1;
			}
			elif(ins.first.type==topnd::c_addr)
			{
				ins.type+=2;
			}
			else
			{
				return false;
			}
		}
		else
		{
			if(ins.second.type==topnd::c_imme)
			{
				if(ins.first.type==topnd::c_reg)
				{
					;
				}
				elif(ins.first.type==topnd::c_addr)
				{
					ins.type+=1;
				}
				else
				{
					;
				}
			}
			elif(ins.second.type==topnd::c_reg)
			{
				if(ins.first.type==topnd::c_reg)
				{
					ins.type+=2;
				}
				elif(ins.first.type==topnd::c_addr)
				{
					ins.type+=3;
				}
				else
				{
					return false;
				}
			}
			elif(ins.second.type==topnd::c_addr)
			{
				if(ins.first.type==topnd::c_reg)
				{
					ins.type+=4;
				}
				elif(ins.first.type==topnd::c_addr)
				{
					ins.type+=5;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		return true;
	}

	static rbool a_opnd(tsh& sh,tasm& item,int index,const rbuf<rstr>& v,topnd& o)
	{
		if(v.empty())
		{
			return true;
		}
		if(v.count()==1)
		{
			if(v.top().is_number())
			{
				//123
				o.type=topnd::c_imme;
				o.val=v.top().touint();
			}
			elif(v[0].get_bottom()==r_char('\"'))
			{
				//"abc"
				trans_cstr(v[0]);
				if(index>=item.vstr.count())
				{
					return false;
				}
				item.vstr[index]=v[0];
				o.type=topnd::c_imme;
				o.val=(uint)(item.vstr[index].begin());
			}
			else
			{
				//ebp
				o.type=topnd::c_reg;
				o.off=get_reg_off(sh,v.top());
			}
		}
		elif(v.count()==3)
		{
			if(v.bottom()==rppoptr(c_mbk_l)&&
				v.top()==rppoptr(c_mbk_r))
			{
				//[ebp]
				o.type=topnd::c_addr;
				o.off=get_reg_off(sh,v[1]);
				o.val=0;
			}
			elif(v.bottom()==rppoptr(c_sbk_l)&&
				v.top()==rppoptr(c_sbk_r)&&
				v[1].is_number())//todo:
			{
				o.type=topnd::c_imme;
				o.val=v[1].touint();
			}
			else
			{
				return false;
			}
		}
		elif(v.count()==5)
		{
			//[ebp+2]
			o.type=topnd::c_addr;
			o.off=get_reg_off(sh,v[1]);
			o.val=v[3].touint();
			if(v[2]==rppoptr(c_minus))
			{
				o.val=-o.val;
			}
		}
		elif(v.count()==7&&v[1]==rppoptr(c_addr))
		{
			item.ins.clear();
			return true;
		}
		elif(v.count()==0)
		{
			return true;
		}
		else
		{
			return false;
		}
		if(o.off>=r_size(treg))
		{
			return false;
		}
		return true;
	}

	static void trans_cstr(rstr& src)
	{
		if(src.count()<2)
		{
			return;
		}
		rstr dst;
		for(int i=1;i<src.count()-1;i++)
		{
			if(src[i]==r_char('\\'))
			{
				if(src.get(i+1)==r_char('b'))
				{
					dst+=r_char('\b');
				}
				elif(src.get(i+1)==r_char('n'))
				{
					dst+=r_char('\n');
				}
				elif(src.get(i+1)==r_char('r'))
				{
					dst+=r_char('\r');
				}
				elif(src.get(i+1)==r_char('\0'))
				{
					dst+=r_char('\0');
				}
				elif(src.get(i+1)==r_char('x'))
				{
					uchar ch=(uchar)(rstr::hextodec(
						src.sub(i+2,i+4)).touint());
					dst+=ch;
					i=i+3;
					continue;
				}
				else
				{
					dst+=src.get(i+1);
				}
				i++;
			}
			else
			{
				dst+=src[i];
			}
		}
		src=r_move(dst);
		src.m_buf.push((uchar)0);
	}

	static rbool is_jmp_ins(int type)
	{
		return type==tkey::c_jmp||type==tkey::c_jebxz||type==tkey::c_jebxnz;
	}

	static int get_reg_off(tsh& sh,const rstr& s)
	{
		treg reg;
		int ret=r_size(treg);
		if(rppkey(c_eax)==s)
		{
			ret=(uchar*)(&reg.eax)-(uchar*)(&reg);
		}
		elif(rppkey(c_ebx)==s)
		{
			ret=(uchar*)(&reg.ebx)-(uchar*)(&reg);
		}
		elif(rppkey(c_ecx)==s)
		{
			ret=(uchar*)(&reg.ecx)-(uchar*)(&reg);
		}
		elif(rppkey(c_edx)==s)
		{
			ret=(uchar*)(&reg.edx)-(uchar*)(&reg);
		}
		elif(rppkey(c_esi)==s)
		{
			ret=(uchar*)(&reg.esi)-(uchar*)(&reg);
		}
		elif(rppkey(c_edi)==s)
		{
			ret=(uchar*)(&reg.edi)-(uchar*)(&reg);
		}
		elif(rppkey(c_esp)==s)
		{
			ret=(uchar*)(&reg.esp)-(uchar*)(&reg);
		}
		elif(rppkey(c_ebp)==s)
		{
			ret=(uchar*)(&reg.ebp)-(uchar*)(&reg);
		}
		elif(rppkey(c_eip)==s)
		{
			ret=(uchar*)(&reg.eip)-(uchar*)(&reg);
		}
		return ret;
	}

	static void clear_reg(treg& reg)
	{
		reg.eax=0;
		reg.ebx=0;
		reg.ebp=0;
		reg.esi=0;
	}

	static void print_reg(treg& reg)
	{
		rstr s;
		s=rstr("ecx: ")+rstr(reg.ecx);
		s.printl();
	}

	static tfunc* find_func(tsh& sh,rstr name)
	{
		rmutex_t mutex_t(sh.m_mutex);
		rbuf<tword> vword;
		ifn(yword::analyse(sh,name,vword,null))
		{
			return 0;
		}
		yclasstl::combine_quote(sh,vword);
		combine_template(sh,vword);
		combine_mbk(sh,vword);
		rbuf<rstr> vstr=ybase::vword_to_vstr(vword);
		tclass* ptci;
		rbuf<rstr> param;
		rstr fname;
		if(vstr.get(1)==rppoptr(c_dot))
		{
			ptci=yfind::class_search(sh,vstr.get(0));
			param=vstr.sub(4,vstr.count()-1);
			fname=vstr.get(2);
		}
		else
		{
			ptci=sh.m_main;
			param=vstr.sub(2,vstr.count()-1);
			fname=vstr.get(0);
		}
		if(ptci==null)
		{
			return null;
		}
		rbuf<rbuf<rstr> > vparam=r_split_a<rstr>(param,rstr(","));
		param.clear();
		for(int i=0;i<vparam.count();i++)
		{
			param.push(rstr::join<rstr>(vparam[i],rstr()));
		}
		if(param.empty())
		{
			return yfind::func_search(*ptci,fname);
		}
		else
		{
			return yfind::func_search_same(*ptci,fname,param);
		}
	}
	
	//查找内部函数
	static uint find_func_bin(tsh& sh,rstr name)
	{
		tfunc* ptfi=find_func(sh,name);
		if(ptfi==null)
		{
			return 0;
		}
		ifn(process(sh,*ptfi))
		{
			return 0;
		}
		return (uint)ptfi->vasm.begin();
	}

	static void combine_mbk(tsh& sh,rbuf<tword>& v)
	{
		for(int i=1;i<v.count();i++)
		{
			ifn(v[i-1]==rppoptr(c_mbk_l)&&v[i]==rppoptr(c_mbk_r))
			{
				continue;
			}
			v[i-1].val+=v[i].val;
			v[i].val.clear();
		}
		ybase::arrange(v);
	}

	static void combine_template(tsh& sh,rbuf<tword>& v)
	{
		for(int i=1;i<v.count();i++)
		{
			if(v[i]!=rppoptr(c_tbk_l))
			{
				continue;
			}
			ifn(v[i-1].is_name())
			{
				continue;
			}
			int right=ybase::find_symm_tbk(sh,v,i);
			if(right>=v.count())
			{
				continue;
			}
			for(int j=i;j<=right;j++)
			{
				v[i-1].val+=v[j].val;
				v[j].clear();
			}
			i=right;
		}
		ybase::arrange(v);
	}
};
