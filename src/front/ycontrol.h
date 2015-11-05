#pragma once

#include "ybase.h"

//替换控制结构并生成语句表，包括return
//tfunc.vword->vsent
struct ycontrol
{
	static rbool proc_func(const tsh& sh,tfunc& tfi)
	{
		ybase::arrange_format(sh,tfi.vword,&tfi);
		if(!replace_switch(sh,tfi.vword))
		{
			return false;
		}
		ifn(replace_ifn(sh,tfi.vword))
		{
			return false;
		}
		ifn(add_semi(sh,tfi.vword))
		{
			return false;
		}
		ifn(auto_bbk(sh,tfi.vword))
		{
			return false;
		}
		ifn(replace_for_to(sh,tfi.vword))
		{
			return false;
		}
		ifn(replace_for(sh,tfi.vword,tfi.id))
		{
			return false;
		}
		ifn(add_else(sh,tfi.vword))
		{
			return false;
		}
		ifn(replace_elif(sh,tfi.vword))
		{
			return false;
		}
		ifn(replace_if(sh,tfi.vword,tfi.id))
		{
			return false;
		}
		if(!del_bbk(sh,tfi.vword))
		{
			return false;
		}
		tfi.vword+=rstr("_func_end");
		tfi.vword+=rppoptr(c_colon);
		tfi.vword+=rppoptr(c_semi);
		ifn(replace_return(sh,tfi.vword,tfi))
		{
			return false;
		}
		rbuf<rbuf<tword> > vsent=r_split_a<tword>(
			tfi.vword,tword(rppoptr(c_semi)));
		for(int i=0;i<vsent.count();i++)
		{
			tsent sent;
			sent.vword=vsent[i];
			tfi.vsent+=sent;
		}
		return true;
	}

	static rbool del_bbk(const tsh& sh,rbuf<tword>& v)
	{
		for(int i=0;i<v.count();i++)
		{
			if(v[i].val!=rppoptr(c_bbk_l))
			{
				continue;
			}
			int left=i;
			int right=ybase::find_symm_bbk(sh,v,left);
			if(right>=v.count())
			{
				rpperror(v.get(i),"miss }");
				return false;
			}
			v[left].val.clear();
			v[right].val.clear();
		}
		ybase::arrange(v);
		return true;
	}

	static rbool add_semi(const tsh& sh,rbuf<tword>& v)
	{
		rbuf<tword> result;
		for(int i=0;i<v.count();i++)
		{
			if(v[i]==rppoptr(c_bbk_l)||v[i]==rppoptr(c_bbk_r))
			{
				result+=v[i];
				continue;
			}
			if(v[i]==rppkey(c_if)||v[i]==rppkey(c_else)||
				v[i]==rppkey(c_for)||v[i]==rppkey(c_switch)||
				v[i]==rppkey(c_case))
			{
				int end=get_sent_end(v,i);
				if(end>=v.count())
				{
					return false;
				}
				result+=v.sub(i,end+1);
				i=end;
				continue;
			}
			int end=get_sent_end(v,i);
			if(end>=v.count())
			{
				return false;
			}
			result+=v.sub(i,end+1);
			if(v[end]!=rppoptr(c_semi))
			{
				tword item=v[end];
				item.val=rppoptr(c_semi);
				result+=tword(item);
			}
			i=end;
		}
		v=result;
		return true;
	}

	static int get_sent_end(rbuf<tword>& v,int begin)
	{
		for(int i=begin+1;i<v.count();i++)
		{
			if(v[i].pos!=v[begin].pos)
			{
				return i-1;
			}
		}
		ifn(v.empty())
		{
			return v.count()-1;
		}
		return v.count();
	}

	static rbool replace_ifn(const tsh& sh,rbuf<tword>& v)
	{
		for(int i=0;i<v.count();i++)
		{
			if(v[i].val!=rppkey(c_ifn))
			{
				continue;
			}
			int pos=get_sent_end(v,i);
			if(pos>=v.count())
			{
				rpperror(v[i]);
				return false;
			}
			rbuf<tword> vcond=v.sub(i+1,pos+1);
			ybase::clear_word_val(v,i,pos+1);
			v[i].multi.push("if");
			v[i].multi.push("!");
			v[i].multi.push("(");
			v[i].multi+=ybase::vword_to_vstr(vcond);
			v[i].multi.push(")");
			i=pos;
		}
		ybase::arrange(v);
		return true;
	}

	//switch中不需要break
	static rbool replace_switch(const tsh& sh,rbuf<tword>& v)
	{
		for(int i=0;i<v.count();i++)
		{
			if(v[i].val!=rppkey(c_switch))
			{
				continue;
			}
			int pos=get_sent_end(v,i)+1;
			if(pos>=v.count())
			{
				rpperror(v[i]);
				return false;
			}
			rbuf<tword> vcond=v.sub(i+1,pos);
			ybase::clear_word_val(v,i,pos);
			if(v[pos]==rppoptr(c_bbk_l))//只跳过不清除
			{
				pos++;
			}
			rbool first=true;
			int left;
			int right;
			while(pos<v.count())
			{
				if(v[pos]!=rppkey(c_case)&&v[pos]!=rppkey(c_else))
				{
					break;
				}
				left=get_sent_end(v,pos)+1;
				if(left>=v.count())
				{
					rpperror(v[pos]);
					return false;
				}
				if(v.get(left)==rppoptr(c_bbk_l))
				{
					right=ybase::find_symm_bbk(sh,v,left);
					if(right>=v.count())
					{
						rpperror(v[pos]);
						return false;
					}
				}
				else
				{
					right=get_low_end(v,left,pos);
					if(right>=v.count())
					{
						rpperror(v[pos]);
						return false;
					}
					if(left>right)
					{
						rpperror(v[pos]);
						return false;
					}
				}
				rbuf<tword> val;
				if(v[left-1]==rppoptr(c_colon))
				{
					val=v.sub(pos+1,left-1);
				}
				else
				{
					val=v.sub(pos+1,left);
				}
				if(v[pos]==rppkey(c_case))
				{
					if(first)
					{
						v[pos].multi.push(rppkey(c_if));
						first=false;
					}
					else
					{
						v[pos].multi.push(rppkey(c_else));
						v[pos].multi.push(rppkey(c_if));
					}
					v[pos].multi+=ybase::vword_to_vstr(val);
					v[pos].multi.push(rppoptr(c_equal_equal));
					v[pos].multi.push(rppoptr(c_sbk_l));
					v[pos].multi+=ybase::vword_to_vstr(vcond);
					v[pos].multi.push(rppoptr(c_sbk_r));
				}
				else
				{
					v[pos].multi.push(rppkey(c_else));
				}
				ybase::clear_word_val(v,pos,left);
				pos=right+1;
			}
		}
		ybase::arrange(v);
		return true;
	}

	static rbool auto_bbk(const tsh& sh,rbuf<tword>& v)
	{
		for(int i=0;i<c_rpp_deep;i++)
		{
			if(!auto_bbk_one(sh,v))
			{
				return false;
			}
			rbool need=ybase::arrange(v);
			ybase::arrange_format(sh,v,null);
			if(!need)
			{
				return true;
			}
		}
		rpperror("auto bbk too deep");
		return false;
	}

	static rbool auto_bbk_one(const tsh& sh,rbuf<tword>& v)
	{
		for(int i=0;i<v.count();i++)
		{
			int begin;
			if(v[i].val==rppkey(c_else)&&
				v.get(i+1).val!=rppkey(c_if))
			{
				begin=i+1;
			}
			elif(v[i].val==rppkey(c_if)||
				v[i].val==rppkey(c_for))
			{
				begin=get_sent_end(v,i)+1;
			}
			else
			{
				continue;
			}
			if(begin>=v.count())
			{
				rpperror(v[i],"miss { or miss tab");
				return false;
			}
			if(v[begin].val==rppoptr(c_bbk_l))
			{
				continue;
			}
			int end=get_low_end(v,begin,i);
			if(end>=v.count())
			{
				rpperror(v[i],"miss { or miss tab");
				return false;
			}
			if(begin>end)
			{
				rpperror(v[i],"miss { or miss tab");
				return false;
			}
			v[begin].multi.push(rppoptr(c_bbk_l));
			v[begin].multi.push(v[begin].val);
			v[begin].val.clear();
			ifn(v[end].val.empty())
			{
				v[end].multi.push(v[end].val);
			}
			v[end].multi.push(rppoptr(c_bbk_r));
			v[end].val.clear();
			return true;
		}
		return true;
	}

	static rbool replace_return(const tsh& sh,rbuf<tword>& v,const tfunc& tfi)
	{
		rbuf<tword> result;
		for(int i=0;i<v.count();i++)
		{
			if(v[i]!=rppkey(c_return))
			{
				result.push(v[i]);
				continue;
			}
			int pos=v.find(rppoptr(c_semi),i);
			if(pos>=v.count())
			{
				return false;
			}
			rbuf<tword> sent=v.sub(i+1,pos);
			ifn(sent.empty())
			{
				if(ybase::is_quote(tfi.retval.type))
				{
					//todo: 应放在后端处理
					if(sh.m_mode==tsh::c_gpp)
					{
						//result+=rppkey(c_s_ret);
						//result+=rppoptr(c_equal);
						result+=sent;
						result+=rppoptr(c_semi);
					}
					else
					{
						result+=rppoptr(c_addr);
						result+=rppoptr(c_sbk_l);
						result+=sent;
						result+=rppoptr(c_sbk_r);
						result+=rppoptr(c_semi);

						result+=rppkey(c_mov);
						result+=rppoptr(c_mbk_l);
						result+=rppkey(c_ebp);
						result+=rppoptr(c_plus);
						result+=rppkey(c_s_off);
						result+=tfi.retval.name;
						result+=rppoptr(c_mbk_r);
						result+=rppoptr(c_comma);
						result+=rppkey(c_ebx);
						result+=rppoptr(c_semi);
					}
				}
				else
				{
					result+=tfi.retval.name;
					result+=rppoptr(c_sbk_l);
					result+=rppoptr(c_sbk_l);
					result+=sent;
					result+=rppoptr(c_sbk_r);
					result+=rppoptr(c_sbk_r);
					result+=rppoptr(c_semi);
				}
			}
			result+=rppkey(c_jmp);
			result+=rstr("_func_end");
			result+=rppoptr(c_semi);
			i=pos;
		}
		v=result;
		return true;
	}

	//获取低一层结尾语句的位置
	static int get_low_end(rbuf<tword>& v,int begin,int head)
	{
		if(v.get(begin).val.empty())
		{
			return v.count();
		}
		for(int i=begin+1;i<v.count();i++)
		{
			if(ybase::get_word_tab(v[i])<=ybase::get_word_tab(v[head]))
			{
				return i-1;
			}
		}
		ifn(v.empty())
		{
			return v.count()-1;
		}
		return v.count();
	}

	//fixme fot..to条件带括号有问题
	static rbool replace_for_to(const tsh& sh,rbuf<tword>& v)
	{
		for(int i=0;i<v.count();i++)
		{
			if(v[i].val!=rppkey(c_for))
			{
				continue;
			}
			int cond_end=v.find(rppoptr(c_bbk_l),i);
			if(cond_end>=v.count())
			{
				return false;
			}
			rbuf<tword> vcond=v.sub(i+1,cond_end);
			int topos=r_find_a<tword>(vcond,tword(rppkey(c_to)));
			if(topos<vcond.count())
			{
				int assignpos=r_find_a<tword>(
					vcond,tword(rppoptr(c_equal)));
				if(assignpos==vcond.count())
				{
					rpperror(v.get(i),"for to miss assign optr");
					return false;
				}
				rstr name=vcond.get(assignpos-1).val;
				if(name.empty())
				{
					rpperror(v.get(i),"for to miss assign name");
					return false;
				}
				v[i].multi.push(rstr("for"));
				v[i].multi.push(rstr("("));
				for(int j=0;j<topos;j++)
				{
					v[i].multi.push(vcond[j].val);
				}
				v[i].multi.push(rppoptr(c_semi));
				v[i].multi.push(name);
				v[i].multi.push(rstr("<="));
				for(int j=topos+1;j<vcond.count();j++)
				{
					v[i].multi.push(vcond[j].val);
				}
				v[i].multi.push(rppoptr(c_semi));
				v[i].multi.push(rstr("++"));
				v[i].multi.push(name);
				v[i].multi.push(rstr(")"));
				ybase::clear_word_val(v,i,cond_end);
				continue;
			}
			int inpos=r_find_a<tword>(vcond,tword(rppkey(c_in)));
			if(inpos<vcond.count())
			{
				rstr var=vcond.get(0).val;
				rbuf<rstr> vset=ybase::vword_to_vstr(vcond.sub(2));
				if(var.empty()||vset.empty())
				{
					rpperror(v.get(i),"for in miss name or set");
					return false;
				}
				ybase::clear_word_val(v,i,cond_end);
				v[i].multi.push(rstr("for"));
				v[i].multi.push(rstr("("));
				v[i].multi+=var;
				v[i].multi+=rstr("=");
				v[i].multi+=rstr("0");
				v[i].multi+=rstr(";");
				v[i].multi+=var;
				v[i].multi+=rstr("<");
				v[i].multi+=vset;
				v[i].multi+=rstr(".");
				v[i].multi+=rstr("count");
				v[i].multi+=rstr("(");
				v[i].multi+=rstr(")");
				v[i].multi+=rstr(";");
				v[i].multi+=rstr("++");
				v[i].multi+=var;
				v[i].multi+=rstr(")");
			}
		}
		ybase::arrange(v);
		return true;
	}

	static rbool replace_for(const tsh& sh,rbuf<tword>& v,int& id)
	{
		for(int i=0;i<v.count();i++)
		{
			if(v[i]!=rppkey(c_for))
			{
				continue;
			}
			int left=v.find(rppoptr(c_bbk_l),i);
			if(left>=v.count())
			{
				return false;
			}
			int right=ybase::find_symm_bbk(sh,v,left);
			if(right>=v.count())
			{
				return false;
			}
			rbuf<rbuf<tword> > vcond;
			if(v.get(i+1)==rppoptr(c_sbk_l)&&v.get(left-1)==rppoptr(c_sbk_r))
			{
				vcond=r_split_e_a<tword>(
					v.sub(i+2,left-1),tword(rppoptr(c_semi)));
			}
			else
			{
				vcond=r_split_e_a<tword>(
					v.sub(i+1,left),tword(rppoptr(c_semi)));
			}
			if(vcond.empty())
			{
				vcond+=rbuf<tword>(1,tword(rstr("1")));//todo:
			}
			if(vcond.count()==1)
			{
				vcond.push(rbuf<tword>());
				vcond.push_front(rbuf<tword>());
			}
			if(vcond.count()!=3)
			{
				return false;
			}
			int cur_id=id;
			id++;
			rbuf<tword> center=v.sub(left+1,right);
			ifn(replace_break_continue(sh,center,cur_id))
			{
				return false;
			}
			rbuf<tword> result;
			result+=v.sub(0,i);
			result+=vcond[0];
			result+=rppoptr(c_semi);
			result+=rstr("_for_judge_")+cur_id;
			result+=rppoptr(c_colon);
			result+=rppoptr(c_semi);
			result+=vcond[1];
			result+=rppoptr(c_semi);
			result+=rstr("jebxz");
			result+=rstr("_for_end_")+cur_id;
			result+=rppoptr(c_semi);
			ifn(replace_for(sh,center,id))
			{
				return false;
			}
			result+=center;
			result+=rstr("_for_add_")+cur_id;
			result+=rppoptr(c_colon);
			result+=rppoptr(c_semi);
			result+=vcond[2];
			result+=rppoptr(c_semi);
			result+=rstr("jmp");
			result+=rstr("_for_judge_")+cur_id;
			result+=rppoptr(c_semi);
			result+=rstr("_for_end_")+cur_id;
			result+=rppoptr(c_colon);
			result+=rppoptr(c_semi);
			center=v.sub(right+1);
			ifn(replace_for(sh,center,id))
			{
				return false;
			}
			result+=center;
			v=result;
			return true;
		}
		return true;
	}

	static rbool replace_break_continue(const tsh& sh,rbuf<tword>& v,int cur_id)
	{
		rbuf<tword> result;
		for(int i=0;i<v.count();i++)
		{
			if(v[i]==rppkey(c_for))
			{
				int left=v.find(rppoptr(c_bbk_l),i);
				if(left>=v.count())
				{
					return false;
				}
				int right=ybase::find_symm_bbk(sh,v,left);
				if(right>=v.count())
				{
					return false;
				}
				result+=v.sub(i,right+1);
				i=right;
			}
			elif(v[i]==rppkey(c_break))
			{
				result+=rstr("jmp");
				result+=rstr("_for_end_")+cur_id;
			}
			elif(v[i]==rppkey(c_continue))
			{
				result+=rstr("jmp");
				result+=rstr("_for_add_")+cur_id;
			}
			else
			{
				result+=v[i];
			}
		}
		v=result;
		return true;
	}

	static rbool add_else(const tsh& sh,rbuf<tword>& v)
	{
		for(int i=0;i<v.count();i++)
		{
			if(v[i]!=rppkey(c_if))
			{
				continue;
			}
			int left=v.find(rppoptr(c_bbk_l),i);
			if(left>=v.count())
			{
				return false;
			}
			int right=ybase::find_symm_bbk(sh,v,left);
			if(right>=v.count())
			{
				return false;
			}
			if(v.get(right+1)==rppkey(c_else))
			{
				continue;
			}
			rbuf<tword> result;
			result+=v.sub(0,left+1);
			rbuf<tword> temp=v.sub(left+1,right);
			ifn(add_else(sh,temp))
			{
				return false;
			}
			result+=temp;
			result+=rppoptr(c_bbk_r);
			result+=rppkey(c_else);
			result+=rppoptr(c_bbk_l);
			result+=rppoptr(c_bbk_r);
			temp=v.sub(right+1);
			ifn(add_else(sh,temp))
			{
				return false;
			}
			result+=temp;
			v=result;
			return true;
		}
		return true;
	}

	static rbool replace_elif(const tsh& sh,rbuf<tword>& v)
	{
		for(int i=0;i<v.count();i++)
		{
			if(v[i]!=rppkey(c_if))
			{
				continue;
			}
			int left=v.find(rppoptr(c_bbk_l),i);
			if(left>=v.count())
			{
				return false;
			}
			int right=ybase::find_symm_bbk(sh,v,left);
			if(right>=v.count())
			{
				return false;
			}
			if(v.get(right+1)!=rppkey(c_else)||
				v.get(right+2)!=rppkey(c_if))
			{
				continue;
			}
			rbuf<tword> result;
			result+=v.sub(0,left+1);
			rbuf<tword> temp=v.sub(left+1,right);
			ifn(replace_elif(sh,temp))
			{
				return false;
			}
			result+=temp;
			int first=v.find(rppoptr(c_bbk_l),right+2);
			if(first>=v.count())
			{
				return false;
			}
			int second=ybase::find_symm_bbk(sh,v,first);
			if(second>=v.count())
			{
				return false;
			}
			while(v.get(second+1)==rppkey(c_else))
			{
				if(v.get(second+2)==rppoptr(c_bbk_l))
				{
					first=second+2;
					second=ybase::find_symm_bbk(sh,v,first);
					break;
				}
				first=v.find(rppoptr(c_bbk_l),second+2);
				second=ybase::find_symm_bbk(sh,v,first);
			}
			result+=rppoptr(c_bbk_r);
			result+=rppkey(c_else);
			result+=rppoptr(c_bbk_l);
			temp=v.sub(right+2,second+1);
			ifn(replace_elif(sh,temp))
			{
				return false;
			}
			result+=temp;
			result+=rppoptr(c_bbk_r);
			temp=v.sub(second+1);
			ifn(replace_elif(sh,temp))
			{
				return false;
			}
			result+=temp;
			v=result;
			return true;
		}
		return true;
	}

	static rbool replace_if(const tsh& sh,rbuf<tword>& v,int& id)
	{
		for(int i=0;i<v.count();i++)
		{
			if(v[i]!=rppkey(c_if))
			{
				continue;
			}
			int left=v.find(rppoptr(c_bbk_l),i);
			if(left>=v.count())
			{
				return false;
			}
			int right=ybase::find_symm_bbk(sh,v,left);
			if(right>=v.count())
			{
				return false;
			}
			if(v.get(right+1)!=rppkey(c_else))
			{
				continue;
			}
			rbuf<tword> result;
			int cur_id=id;
			id++;
			result+=v.sub(0,i);
			result+=v.sub(i+1,left);
			result+=rppoptr(c_semi);
			result+=rstr("jebxz");
			result+=rstr("_else_start_")+cur_id;
			result+=rppoptr(c_semi);
			rbuf<tword> temp=v.sub(left+1,right);
			ifn(replace_if(sh,temp,id))
			{
				return false;
			}
			result+=temp;
			result+=rstr("jmp");
			result+=rstr("_else_end_")+cur_id;
			result+=rppoptr(c_semi);
			result+=rstr("_else_start_")+cur_id;
			result+=rppoptr(c_colon);
			result+=rppoptr(c_semi);
			left=right+2;
			right=ybase::find_symm_bbk(sh,v,left);
			temp=v.sub(left+1,right);
			ifn(replace_if(sh,temp,id))
			{
				return false;
			}
			result+=temp;
			result+=rstr("_else_end_")+cur_id;
			result+=rppoptr(c_colon);
			result+=rppoptr(c_semi);
			temp=v.sub(right+1);
			ifn(replace_if(sh,temp,id))
			{
				return false;
			}
			result+=temp;
			v=result;
			return true;
		}
		return true;
	}
};
