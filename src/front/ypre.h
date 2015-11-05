#pragma once

#include "yword.h"
#include "ysuper.h"

//预处理、获取所有文件
//词表->m_file.vword
struct ypre
{
	static rbool process(tsh& sh)
	{
		ifn(auto_import(sh))
		{
			return false;
		}
		if(sh.m_main_cont!=null)
		{
			tfile item;
			item.name=sh.m_main_file;
			item.cont=sh.m_main_cont;
			sh.m_file.insert(item);
		}
		else
		{
			if(!read_file(sh,sh.m_main_file))
			{
				rpperror("can't read main file "+
					sh.m_main_file.torstr());
				return false;
			}
		}
		if(sh.m_mode==tsh::c_jit)
		{
			sh.m_vdefine.insert_c(tmac(rstr("_JIT")));
		}
		elif(sh.m_mode==tsh::c_interpret)
		{
			sh.m_vdefine.insert_c(tmac(rppkey(c_rvm)));
		}
		elif(sh.m_mode==tsh::c_win)
		{
			sh.m_vdefine.insert_c(tmac(rstr("_WIN")));
			sh.m_vdefine.insert_c(tmac(rstr("_NASM")));
		}
		elif(sh.m_mode==tsh::c_grub)
		{
			sh.m_vdefine.insert_c(tmac(rstr("_GRUB")));
		}
		elif(sh.m_mode==tsh::c_gpp)
		{
			sh.m_vdefine.insert_c(tmac(rstr("_GPP")));
		}
		sh.m_vdefine.insert_c(tmac(rstr("_RPP")));
#ifdef _MSC_VER
		sh.m_vdefine.insert_c(tmac(rstr("_WIN32")));
#endif
		if(!obtain_all_file(sh))
		{
			return false;
		}
		for(tfile* p=sh.m_file.begin();p!=sh.m_file.end();p=sh.m_file.next(p))
		{
			if(!obtain_def(sh,sh.m_vdefine,p->vword))
			{
				return false;
			}
		}
		for(tfile* p=sh.m_file.begin();p!=sh.m_file.end();p=sh.m_file.next(p))
		{
			if(!ifdef_replace(sh,sh.m_vdefine,p->vword))
			{
				return false;
			}
		}
		for(tfile* p=sh.m_file.begin();p!=sh.m_file.end();p=sh.m_file.next(p))
		{
			ifn(def_replace(sh,sh.m_vdefine,p->vword))
			{
				return false;
			}
		}
		return true;
	}

	static rbool auto_import(tsh& sh)
	{
		ifn(tconf::c_auto_import)
		{
			return true;
		}
		if(sh.m_mode==tsh::c_gpp)
		{
			return read_file(sh,ybase::get_rpp_dir_w()+rstrw("src/xlib/gpp/xbase.rpp"));
		}
		return read_file(sh,ybase::get_rpp_dir_w()+rstrw("src/xlib/rpp/xbase.rpp"));
	}

	static rbool read_file(tsh& sh,const rstrw& name)
	{
		if(name.empty())
		{
			return false;
		}
		ifn(rfile::exist(name))
		{
			return false;
		}
		tfile item;
		item.name=name;
		if(sh.m_file.exist(item))
		{
			return true;
		}
		item.cont=rfile::read_all_n(name);
		if(item.cont.empty())
		{
			return false;
		}
		item.cont=rcode::to_utf8_txt(item.cont).sub(3);
		sh.m_file.insert(item);
		return true;
	}

	static rbool str_analyse(const tsh& sh,rstr& src,rbuf<tword>& dst,const tfile* pfile)
	{
		if(!yword::analyse(sh,src,dst,pfile))
		{
			return false;
		}
		const_replace(dst);
		combine_double(dst);
		combine_float(dst);
		key_replace(sh,dst);
		if(pfile!=null&&rdir::get_suffix_w(pfile->name)!=rstr("rpp"))
		{
			extern_replace(sh,dst);
			this_replace(sh,dst);
		}
		return true;
	}

	static rbool obtain_all_file(tsh& sh)
	{
		while(true)
		{
			int cur=sh.m_file.count();
			rbuf<rstrw> vname;
			for(tfile* p=sh.m_file.begin();
				p!=sh.m_file.end();p=sh.m_file.next(p))
			{
				count_tab(*p);
				if(!str_analyse(sh,p->cont,p->vword,p))
				{
					return false;
				}
				//仅编译器内部使用
				ifn(ifdef_replace(sh,sh.m_vdefine,p->vword))
				{
					return false;
				}
				if(!obtain_name(sh,vname,p->vword,*p))
				{
					rpperror(rstr("obtain error ")+
						ybase::get_file_name(p));
					return false;
				}
			}
			if(!import_file(sh,vname))
			{
				rpperror("import error");
				return false;
			}
			if(cur==sh.m_file.count())
			{
				return true;
			}
		}
	}

	static rbool import_file(tsh& sh,const rbuf<rstrw>& vname)
	{
		for(int i=0;i<vname.count();i++)
		{
			ifn(read_file(sh,vname[i]))
			{
				rpperror("can't read file "+vname[i].torstr());
				return false;
			}
		}
		return true;
	}

	static rbool obtain_name(const tsh& sh,rbuf<rstrw>& vname,rbuf<tword>& v,const tfile& f)
	{
		rstrw exe_dir=rdir::get_exe_dir_w();
		for(int i=0;i<v.count();i++)
		{
			if(v[i].val!=rppkey(c_import))
			{
				continue;
			}
			if(v.get(i-1).val==rppoptr(c_sharp))
			{
				v[i-1].clear();
			}
			v[i].val.clear();
			rstrw name;
			if(v.get(i+1).is_cstr())
			{
				name=v.get(i+1).val;
				v[i+1].clear();
			}
			else
			{
				//处理不带引号的import
				int j;
				for(j=i+1;j<v.count();j++)
				{
					if(v[j].pos.line!=v[i].pos.line)
					{
						break;
					}
					name+=rstrw(v[j].val);
					v[j].val.clear();
				}
				name=rstrw(ybase::add_quote(name.torstr()));
			}
			if(name.count()<3)
			{
				return false;
			}
			name.pop();
			name.pop_front();
			name=rdir::dir_std_w(name);
			rstrw temp=get_abs_name(rdir::get_prev_dir_w(f.name),name);
			ifn(rfile::exist(temp))
			{
				ifn(get_file(sh,vname,name,temp))
				{
					return false;
				}
			}
			if(!vname.exist(temp)&&!sh.m_file.exist(tfile(temp)))
			{
				vname.push(temp);
			}
		}
		ybase::arrange(v);
		return true;
	}

	static rbool get_file(const tsh& sh,const rbuf<rstrw>& vname,
		const rstrw& name,rstrw& abs_name)
	{
		for(int i=0;i<sh.m_path.count();i++)
		{
			//todo: 直接import
			abs_name=get_abs_name(sh.m_path[i],name);
			if(rfile::exist(abs_name))
			{
				return true;
			}
		}
		return false;
	}

	static rstrw get_abs_name(rstrw path,const rstrw& name)
	{
		if(path.empty())
		{
			return rstrw();
		}
		rbuf<rstrw> temp=r_split<rstrw>(name,rstrw("/"));
		if(temp.empty())
		{
			return rstrw();
		}
		for(int i=0;i<temp.count();i++)
		{
			if(temp[i]==rstrw(".."))
			{
				path=rdir::get_prev_dir_w(path);
			}
			elif(temp[i]==rstrw("."))
			{
				;
			}
			else
			{
				path+=temp[i];
				if(i!=temp.count()-1)
				{
					path+=rstrw("/");
				}
			}
		}
		return r_move(path);
	}

	static rstrw get_abs_path(const rstrw& s)
	{
		if(s.sub(0,2)==rstrw("//")||s.sub(1,3)==rstrw(":/"))
		{
			return s;
		}
		return get_abs_name(rdir::get_cur_dir_w(),s);
	}

	static void this_replace(const tsh& sh,rbuf<tword>& v)
	{
		for(int i=0;i<v.count();i++)
		{
			if(v[i]!=rppkey(c_this))
			{
				continue;
			}
			if(v.get(i+1)==rppoptr(c_arrow_r))
			{
				v[i+1].val=rppoptr(c_dot);
				continue;
			}
			v[i].val.clear();
			v[i].multi+=rppoptr(c_sbk_l);
			v[i].multi+=rppoptr(c_addr);
			v[i].multi+=rppkey(c_this);
			v[i].multi+=rppoptr(c_sbk_r);
		}
		ybase::arrange(v);
	}

	static void extern_replace(const tsh& sh,rbuf<tword>& v)
	{
		for(int i=0;i<v.count();i++)
		{
			if(v[i]!=rppkey(c_extern))
			{
				continue;
			}
			int pos=v.find(rppoptr(c_semi),i);
			if(pos<v.count())
			{
				ybase::clear_word_val(v,i,pos+1);
			}
		}
		ybase::arrange(v);
	}

	static void key_replace(const tsh& sh,rbuf<tword>& v)
	{
		for(int i=0;i<v.count();i++)
		{
			if(v[i].val==rppkey(c_include))
			{
				v[i].val=rppkey(c_import);
			}
			//这里不能用超级宏实现，因为有带冒号和不带冒号两种
			if(v[i].val==rppkey(c_private))
			{
				v[i].clear();
				if(rppoptr(c_colon)==v.get(i+1).val)
				{
					v[i+1].clear();
				}
			}
			if(v[i].val==rppkey(c_public))
			{
				v[i].clear();
				if(rppoptr(c_colon)==v.get(i+1).val)
				{
					v[i+1].clear();
				}
			}
			if(v[i].val==rppkey(c_protected))
			{
				v[i].clear();
				if(rppoptr(c_colon)==v.get(i+1).val)
				{
					v[i+1].clear();
				}
			}
			if(v[i].val==rppoptr(c_sharp)&&v.get(i+1).val=="pragma")
			{
				if(i+2<v.count())
				{
					v[i+2].clear();
				}
				v[i].clear();
				v[i+1].clear();
			}
		}
		ybase::arrange(v);
	}

	static rbool def_replace_one(const tsh& sh,rbuf<tword>& v,const rset<tmac>& vmac)
	{
		tmac item;
		for(int i=0;i<v.count();i++)
		{
			item.name=v[i].val;
			tmac* p=vmac.find(item);
			if(p==null)
			{
				continue;
			}
			if(p->is_super)
			{
				ifn(ysuper::replace_item(sh,v,i,*p))
				{
					return false;
				}
			}
			else
			{
				v[i].val.clear();
				v[i].multi=p->vstr;
			}
		}
		return true;
	}

	static rbool def_replace(const tsh& sh,const rset<tmac>& vdefine,rbuf<tword>& v)
	{
		for(int i=0;i<c_rpp_deep;i++)
		{
			ifn(def_replace_one(sh,v,vdefine))
			{
				return false;
			}
			ifn(ybase::arrange(v))
			{
				return true;
			}
		}
		return false;
	}

	static rbool obtain_def(const tsh& sh,rset<tmac>& vdefine,rbuf<tword>& v)
	{
		for(int i=0;i<v.count();i++)
		{
			if(v[i].val!=rppkey(c_define))
			{
				continue;
			}
			if(v.get(i-1).val==rppoptr(c_sharp))
			{
				v[i-1].clear();
			}
			if(v.get(i+1).is_name()&&v.get(i+2)==rppoptr(c_sbk_l))
			{
				//todo:暂时替换成mac
				v[i].val=rppkey(c_mac);
				continue;
			}
			if(v.get(i+1)=="$")
			{
				int right=i;
				ifn(ysuper::add_super_mac(sh,v,right,vdefine))
				{
					return false;
				}
				ybase::clear_word_val(v,i,right+1);
				i=right;
				continue;
			}
			tmac item;
			item.name=v.get(i+1).val;
			if(item.name.empty())
			{
				rpperror(v[i]);
				return false;
			}
			for(int k=i+2;k<v.count()&&v.get(k).pos==v[i].pos;k++)
			{
				item.vstr.push(v[k].val);
				v[k].clear();
			}
			if(vdefine.exist(item))
			{
				vdefine.erase(item);
				/*rpperror(v.get(i+1),"redefined");
				return false;*/
			}
			vdefine.insert(item);
			v[i+1].clear();
			v[i].clear();
		}
		ybase::arrange(v);
		return true;
	}

	//fixme:
	static rbool ifdef_replace(const tsh& sh,const rset<tmac>& vdefine,rbuf<tword>& v)
	{
		tmac item;
		for(int i=v.count()-1;i>=0;i--)
		{
			if(v[i]!=rppoptr(c_sharp))
			{
				continue;
			}
			rstr key=v.get(i+1).val;
			if(key!=rppkey(c_ifdef)&&
				key!=rppkey(c_ifndef))
			{
				continue;
			}
			if(i+2>=v.count())
			{
				rpperror(v[i],"ifdef");
				return false;
			}
			item.name=v[i+2].val;
			int endpos=r_find_a<tword>(v,tword(rppkey(c_endif)),i+3);
			if(endpos>=v.count())
			{
				rpperror(v[i],"ifdef");
				return false;
			}
			int elsepos=endpos;
			for(int j=i+3;j<endpos;j++)
			{
				if(v[j]==rppoptr(c_sharp)&&
					v.get(j+1)==rppkey(c_else))
				{
					elsepos=j+1;
					break;
				}
			}
			rbool defined=vdefine.exist(item);
			if(key==rppkey(c_ifdef))
			{
				if(defined)
				{
					ybase::clear_word_val(v,elsepos,endpos);
				}
				else
				{
					ybase::clear_word_val(v,i,elsepos);
				}
			}
			else
			{
				if(defined)
				{
					ybase::clear_word_val(v,i,elsepos);
				}
				else
				{
					ybase::clear_word_val(v,elsepos,endpos);
				}
			}
			v[i].clear();
			v[i+1].clear();
			v[i+2].clear();
			v[endpos-1].clear();
			v[endpos].clear();
			v[elsepos-1].clear();
			v[elsepos].clear();
		}
		ybase::arrange(v);
		return true;
	}

	static void combine_double(rbuf<tword>& v)
	{
		for(int i=1;i<v.count()-1;i++)
		{
			if(v[i]=="."&&
				v[i-1].val.is_number()&&
				v[i+1].val.is_number())
			{
				v[i-1].val+=v[i].val;
				v[i-1].val+=v[i+1].val;
				v[i].clear();
				v[i+1].clear();
				i++;
			}
		}
		ybase::arrange(v);
	}

	static void combine_float(rbuf<tword>& v)
	{
		for(int i=0;i<v.count();i++)
		{
			ifn(v[i]=="."&&v.get(i-1).val.is_number())
			{
				continue;
			}
			rstr s=v.get(i+1).val;
			if(s.get_top()!=r_char('f'))
			{
				continue;
			}
			ifn(s.sub(0,s.count()-1).is_number())
			{
				continue;
			}
			v[i].multi+="double";
			v[i].multi+="(";
			v[i].multi+=v[i-1].val+"."+s.sub(0,s.count()-1);
			v[i].multi+=")";
			v[i].multi+=".";
			v[i].multi+="tofloat";
			v[i].val.clear();
			v[i+1].val.clear();
			v[i-1].val.clear();
		}
		ybase::arrange(v);
	}

	static void const_replace(rbuf<tword>& v)
	{
		for(int i=0;i<v.count();i++)
		{
			const_replace(v[i]);
		}
	}

	static void const_replace(tword& word)
	{
		if(word.val.count()>0&&rstr::is_number(word.val[0]))
		{
			if(r_find_a<uchar>(word.val.m_buf,r_char('_'))<word.val.count())
			{
				rstr s;
				for(int i=0;i<word.val.count();i++)
				{
					if(word.val[i]!=r_char('_'))
					{
						s+=word.val[i];
					}
				}
				word.val=r_move(s);
			}
		}
		if(word.val.count()>2)
		{
			if(word.val[0]==r_char('0')&&word.val[1]==r_char('x'))
			{
				word.val=rstr::hextodec(word.val.sub(2));
			}
			if(word.val[0]==r_char('0')&&word.val[1]==r_char('b'))
			{
				word.val=rstr::bintodec(word.val.sub(2));
			}
		}
	}

	static int count_tab_line(const rstr& s)
	{
		int i;
		for(i=0;i<s.count();i++)
		{
			if(s[i]!=r_char(' ')&&s[i]!=0x9)
			{
				break;
			}
		}
		int sum=0;
		for(int k=0;k<i;k++)
		{
			if(s[k]==r_char(' '))
			{
				sum++;
			}
			elif(s[k]==0x9)
			{
				sum+=4;
			}
		}
		return sum/4;
	}

	static void count_tab(tfile& file)
	{
		file.line_list=r_split_e<rstr>(file.cont,rstr("\n"));
		file.line_list.push_front(rstr());//行号从1开始
		for(int i=0;i<file.line_list.count();i++)
		{
			file.tab_list.push(count_tab_line(file.line_list[i]));
		}
	}
};
