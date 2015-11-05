#pragma once

#include "yadd.h"
#include "yexp.h"

//简单替换
struct yrep
{
	static rbool typeof_replace(tsh& sh,tfunc& tfi,tenv env)
	{
		for(int i=0;i<tfi.vsent.count();i++)
		{
			if(!typeof_replace(sh,tfi,tfi.vsent[i],env))
			{
				return false;
			}
		}
		return true;
	}

	static rbool typeof_replace(tsh& sh,tfunc& tfi,tsent& sent,tenv env)
	{
		rbuf<tword>& v=sent.vword;
		for(int i=0;i<v.count();i++)
		{
			if(v[i]!=rppkey(c_typeof))
			{
				continue;
			}
			int left=i+1;
			if(v.get(left)!=rppoptr(c_sbk_l))
			{
				rpperror(sent,"miss (");
				return false;
			}
			int right=ybase::find_symm_sbk(sh,v,left);
			if(right>=v.count())
			{
				rpperror(sent,"miss )");
				return false;
			}
			tsent dst;
			dst.pos=sent.pos;
			dst.vword=v.sub(left+1,right);
			if(dst.vword.count()==1&&yfind::is_class(sh,dst.vword[0].val))
			{
				dst.type=dst.vword[0].val;
			}
			else
			{
				if(!yexp::p_exp(sh,dst,tfi,0,env))
				{
					return false;
				}
			}
			ybase::clear_word_val(v,i,right+1);
			v[i].val=ybase::add_quote(dst.type);
		}
		ybase::arrange(v);
		return true;
	}

	static void fpoint_replace(const tsh& sh,tfunc& tfi)
	{
		for(int i=0;i<tfi.vsent.count();++i)
		{
			fpoint_replace(sh,*tfi.ptci,tfi.vsent[i].vword);
		}
	}

	static void fpoint_replace(const tsh& sh,const tclass& tci,rbuf<tword>& v)
	{
		for(int i=0;i<v.count();i++)
		{
			if(v[i].val!=rppoptr(c_addr))
			{
				continue;
			}
			const tclass* ptci;
			rstr name;
			tfunc* ptfi;
			int left;
			if(v.get(i+2).val==rppoptr(c_dot))
			{
				ptci=yfind::class_search(sh,v.get(i+1).val);
				if(null==ptci)
				{
					continue;
				}
				name=v.get(i+3).val;
				left=i+4;
			}
			else
			{
				name=v.get(i+1).val;
				ptci=&tci;
				ptfi=yfind::func_search(*ptci,name);
				if(null==ptfi)
				{
					ptci=sh.m_main;	
				}
				left=i+2;
			}
			if(v.get(left)!=rppoptr(c_sbk_l))
			{
				ptfi=yfind::func_search(*ptci,name);
				if(null==ptfi)
				{
					continue;
				}
				ybase::clear_word_val(v,i,left);
				v[i].multi=ybase::get_func_declare_lisp(sh,*ptci,*ptfi);
				i=left-1;
			}
			else
			{
				int right=ybase::find_symm_sbk(sh,v,left);
				if(right>=v.count())
				{
					continue;
				}
				rbuf<tsent> vsent;
				ybase::split_param(sh,vsent,v.sub(left+1,right));
				rbuf<rstr> vtype;
				for(int j=0;j<vsent.count();j++)
				{
					vtype.push(vsent[j].vword.get(0).val);
				}
				ptfi=yfind::func_search_same(*ptci,name,vtype);
				if(null==ptfi)
				{
					continue;
				}
				ybase::clear_word_val(v,i,right+1);
				v[i].multi=ybase::get_func_declare_lisp(sh,*ptci,*ptfi);
				i=right;
			}
		}
		ybase::arrange(v);
	}

	static void const_replace(const tsh& sh,rbuf<tsent>& vsent)
	{
		for(int i=0;i<vsent.count();++i)
		{
			if(sh.m_key.is_asm_ins(vsent[i].vword.get_bottom().val))
			{
				continue;
			}
			const_replace(sh,vsent[i].vword);
		}
	}

	static void const_replace(const tsh& sh,rbuf<tword>& v)
	{
		for(int i=0;i<v.count();i++)
		{
			if(!v[i].is_const())
			{
				continue;
			}
			if(v[i].is_cint()&&v.get(i+1).val==rppoptr(c_dot))
			{
				v[i].multi.push(rppkey(c_int));
				v[i].multi.push(rppoptr(c_sbk_l));
				v[i].multi.push(v[i].val);
				v[i].multi.push(rppoptr(c_sbk_r));
				v[i].val.clear();
			}
			elif(v[i].is_cdouble()&&v.get(i+1).val==rppoptr(c_dot))
			{
				v[i].multi.push(rstr("double"));
				v[i].multi.push(rppoptr(c_sbk_l));
				v[i].multi.push(v[i].val);
				v[i].multi.push(rppoptr(c_sbk_r));
				v[i].val.clear();
			}
			elif(v[i].is_cstr()&&
				(v.get(i+1).val==rppoptr(c_dot)||
				yfind::is_rstr_optr(sh,v.get(i+1).val)||
				yfind::is_rstr_optr(sh,v.get(i-1).val)))
			{
				v[i].multi.push(rppkey(c_rstr));
				v[i].multi.push(rppoptr(c_sbk_l));
				v[i].multi.push(v[i].val);
				v[i].multi.push(rppoptr(c_sbk_r));
				v[i].val.clear();
			}
		}
		ybase::arrange(v);
	}

	static void replace_neg(const tsh& sh,rbuf<tsent>& vsent)
	{
		for(int i=0;i<vsent.count();i++)
		{
			replace_neg(sh,vsent[i].vword);
		}
	}

	static void replace_neg(const tsh& sh,rbuf<tword>& v)
	{
		for(int i=0;i<v.count()-1;i++)
		{
			if(v[i]!=rppoptr(c_minus))
			{
				continue;
			}
			if(i==0)
			{
				v[i].val="neg";
				continue;
			}
			if(v[i-1]==rppoptr(c_equal)||
				v[i-1]==rppoptr(c_sbk_l)||
				v[i-1]==rppoptr(c_mbk_l)||
				v[i-1]==rppoptr(c_comma)||
				v[i-1]==rppoptr(c_equal_equal)||
				v[i-1]==rppoptr(c_not_equal))
			{
				v[i].val="neg";
			}
		}
	}

	static rbool size_off_to_zero(const tsh& sh,tfunc& tfi)
	{
		for(int i=0;i<tfi.vsent.count();++i)
		{
			if(!size_off_to_zero(sh,tfi.vsent[i].vword))
			{
				rpperror(tfi.vsent[i],"size_off_to_zero error");
				return false;
			}
		}
		return true;
	}

	static rbool size_off_to_zero(const tsh& sh,rbuf<tword>& v)
	{
		for(int i=0;i<v.count();i++)
		{
			if(v[i].val!=rppkey(c_sizeof)&&v[i].val!=rppkey(c_s_off))
			{
				continue;
			}
			if(i+1>=v.count())
			{
				return false;
			}
			if(v.get(i+1).val!=rppoptr(c_sbk_l))
			{
				v[i].multi.push(v[i].val);
				v[i].multi.push(v.get(i+1).val);
				v[i+1].clear();
				v[i].val=rstr("0");
				i++;
			}
			else
			{
				int right=ybase::find_symm_sbk(sh,v,i+1);
				if(right>=v.count())
				{
					return false;
				}
				v[i].multi.push(v[i].val);
				v[i].multi+=ybase::vword_to_vstr(v.sub(i+2,right));
				ybase::clear_word_val(v,i+1,right+1);
				v[i].val=rstr("0");
				i=right;
			}
		}
		ybase::arrange(v);
		return true;
	}

	static rbool size_off_replace(const tsh& sh,tfunc& tfi)
	{
		for(int i=0;i<tfi.vsent.count();++i)
		{
			if(!yrep::size_off_replace(sh,tfi.vsent[i].vword,tfi))
			{
				rpperror(tfi.vsent[i],"size_off_replace error");
				return false;
			}
		}
		return true;
	}

	static rbool size_off_replace(const tsh& sh,rbuf<tword>& v,const tfunc& tfi)
	{
		for(int i=0;i<v.count();i++)
		{
			if(v[i].multi.count()!=2)
			{
				continue;
			}
			rstr name=v[i].multi[1];
			if(v[i].multi[0]==rppkey(c_sizeof))
			{
				if(rppkey(c_s_local)==name)
				{
					v[i].val=yfind::get_func_local_size(tfi);
				}
				elif(rppkey(c_s_param)==name)
				{
					v[i].val=yfind::get_func_param_size(tfi);
				}
				else
				{
					tclass* ptci=yfind::class_search(sh,name);
					if(null==ptci)
					{
						return false;
					}
					v[i].val=ptci->size;
				}
			}
			elif(v[i].multi[0]==rppkey(c_s_off))
			{
				tdata* ptdi=yfind::local_search(tfi,name);
				if(null==ptdi)
				{
					return false;
				}
				v[i].val=ptdi->off;
			}
			else
			{
				continue;
			}
			v[i].multi.clear();
		}
		ifn(sh.m_key.is_asm_ins(v.get_bottom().val))
		{
			return true;
		}
		if(sh.m_mode!=tsh::c_gpp)
		{
			for(int i=1;i<v.count();i++)
			{
				if(!v[i].is_name())
				{
					continue;
				}
				tdata* ptdi=yfind::local_search(tfi,v[i].val);
				if(ptdi==null)
				{
					continue;
				}
				rbuf<rstr> vdst;
				if(v.get(i-1).val==rppoptr(c_addr))
				{
					vdst.push(rstr("ebp"));
					vdst.push(rstr("+"));
					vdst.push(rstr(ptdi->off));
					v[i-1].clear();
				}
				else
				{
					vdst.push(rstr(rppoptr(c_mbk_l)));
					vdst.push(rstr("ebp"));
					vdst.push(rstr("+"));
					vdst.push(rstr(ptdi->off));
					vdst.push(rstr(rppoptr(c_mbk_r)));
				}
				v[i].clear();
				v[i].multi=r_move(vdst);
			}
		}
		ybase::arrange(v);
		return true;
	}

	static rbool local_var_replace(const tsh& sh,tfunc& tfi)
	{
		for(int i=0;i<tfi.vsent.count();++i)
		{
			if(sh.m_key.is_asm_ins(tfi.vsent[i].vword.get_bottom().val))
			{
				continue;
			}
			if(!local_var_replace(sh,tfi.vsent[i].vword,tfi))
			{
				return false;
			}
		}
		ybase::part_vsent(tfi);
		return true;
	}

	static rbool local_var_replace(const tsh& sh,rbuf<tword>& v,tfunc& tfi)
	{
		if(v.count()<2||
			!yfind::is_class_t(sh,v.get(0).val)||
			!v.get(1).is_name())
		{
			return true;
		}
		tdata tdi;
		if(!ymemb::a_data_define(sh,tdi,v))
		{
			return false;
		}
		v[0].val.clear();
		if(v.count()==2)
		{
			v[1].val.clear();//清除未初始化的变量定义
		}
		//如int a(1)这样的定义千万不能重复调用构造函数
		rbool bstruct=v.count()>2&&v[2].val==rppoptr(c_sbk_l);
		ybase::arrange(v);
		//排除重复定义的
		tdata* ptdi=yfind::local_search(tfi,tdi.name);
		if(null==ptdi)
		{
			tfi.local.push(tdi);
		}
		else
		{
			if(ptdi->type!=tdi.type)
			{
				rpperror(v.get(0),"diff type local var redefined");
				return false;
			}
		}
		rbuf<tword> vtemp;
		if(ybase::is_quote(tdi.type)&&v.get(1)==rppoptr(c_equal))
		{
			tdata temp_item;
			temp_item.type="rp<void>";
			temp_item.name=rppkey(c_temp)+rstr(tfi.tid);
			tfi.tid++;
			tfi.local.push(temp_item);

			vtemp+=temp_item.name;
			vtemp+=rppoptr(c_equal);
			vtemp+=rppoptr(c_addr);
			vtemp+=rppoptr(c_sbk_l);
			vtemp+=v.sub(2);
			vtemp+=rppoptr(c_sbk_r);
			vtemp+=rppoptr(c_semi);
			vtemp+=rppkey(c_mov);
			vtemp+=tdi.name;
			vtemp+=rppoptr(c_comma);
			vtemp+=temp_item.name;
			v=r_move(vtemp);
		}
		else
		{
			yadd::add_destructor_func(sh,*tfi.ptci,tdi,vtemp,false);
			if(!bstruct)
			{
				yadd::add_structor_func(sh,tdi,vtemp);
			}
			vtemp+=r_move(v);
			v=r_move(vtemp);
		}
		return true;
	}

	static rbool var_struct_replace(const tsh& sh,const tfunc& tfi)
	{
		for(int i=0;i<tfi.vsent.count();++i)
		{
			if(sh.m_key.is_asm_ins(tfi.vsent[i].vword.get_bottom().val))
			{
				continue;
			}
			if(!var_struct_replace(sh,tfi.vsent[i].vword,tfi))
			{
				rpperror(tfi.vsent[i],"var_struct_replace error");
				return false;
			}
		}
		return true;
	}

	static rbool var_struct_replace(const tsh& sh,rbuf<tword>& v,const tfunc& tfi)
	{
		//暂时只处理a(1)，忽略this.a(1)
		int left=1;
		if(v.get(left).val!=rppoptr(c_sbk_l))
		{
			return true;
		}
		tdata* ptdi=yfind::local_search(tfi,v.get(0).val);
		if(ptdi==null)
		{
			return true;
		}
		if(ybase::is_quote(ptdi->type))
		{
			return true;
		}
		int right=ybase::find_symm_sbk(sh,v,left);
		if(right>=v.count())
		{
			return false;
		}
		if(right!=v.count()-1)
		{
			return true;
		}
		//动态类型和构造函数有歧义
		rbuf<tword> vtemp;
		vtemp.push(tword(ptdi->type));
		vtemp.push(tword(rppoptr(c_dot)));
		vtemp.push(tword(ptdi->type));
		vtemp.push(tword(rppoptr(c_sbk_l)));
		vtemp+=v.sub(0,left);
		rbuf<tword> vsub=v.sub(left+1,right);
		if(!vsub.empty())
		{
			vtemp.push(tword(rppoptr(c_comma)));
			vtemp+=vsub;
		}
		vtemp.push(tword(rppoptr(c_sbk_r)));
		v=r_move(vtemp);
		return true;
	}

	/*
	(int)a
	->
	a.to<int>

	(int)(a.b)
	->
	(a.b).to<int>
	*/
	static rbool trans_replace(const tsh& sh,rbuf<tword>& v)
	{
		for(int i=0;i<v.count();i++)
		{
			if(v[i]!=rppoptr(c_sbk_l))
			{
				continue;
			}
			if(v.get(i+2)!=rppoptr(c_sbk_r))
			{
				continue;
			}
			if(v.get(i-1)==rppkey(c_sizeof)||
				v.get(i-1)==rppkey(c_typeof))
			{
				continue;
			}
			ifn(yfind::is_class(sh,v.get(i+1).val))
			{
				continue;
			}
			if(i+3>=v.count())
			{
				rpperror(v[i],"trans_replace error");
				return false;
			}
			rbuf<tword> src;
			rstr dst_type=v[i+1].val;
			if(v.get(i+3)==rppoptr(c_sbk_l))
			{
				int right=ybase::find_symm_sbk(sh,v,i+3);
				if(right>=v.count())
				{
					rpperror(v[i],"trans_replace error");
					return false;
				}
				src=v.sub(i+3,right+1);
				ybase::clear_word_val(v,i,right+1);
			}
			else
			{
				src+=v[i+3];
				ybase::clear_word_val(v,i,i+4);
			}
			v[i].multi+=ybase::vword_to_vstr(src);
			v[i].multi+=rppoptr(c_dot);
			v[i].multi+=rppkey(c_to);
			v[i].multi+=rppoptr(c_tbk_l);
			v[i].multi+=dst_type;
			v[i].multi+=rppoptr(c_tbk_r);
			if(ybase::arrange(v))
			{
				trans_replace(sh,v);
			}
			return true;
		}
		return true;
	}
};
