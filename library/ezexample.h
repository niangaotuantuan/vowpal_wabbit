#ifndef EZEXAMPLE_H
#define EZEXAMPLE_H

#include <stdio.h>
#include "../vowpalwabbit/vw.h"
#include "../vowpalwabbit/v_array.h"

using namespace std;
typedef uint32_t fid;

struct vw_namespace {
  char namespace_letter;
public: vw_namespace(const char c) : namespace_letter(c) {}
};

class ezexample {
private:
  vw*vw_ref;
  v_array<VW::feature_space> dat;
  vector<fid> past_seeds;
  fid current_seed;
  v_array<feature> current_ns;
  size_t current_name;
  char str[2];
  bool pass_empty;
  string mylabel;
  ezexample(const ezexample & ex);
  ezexample & operator=(const ezexample & ex);

public:

  ezexample(vw*this_vw, bool pe=false) { 
    vw_ref = this_vw;
    current_seed = 0;
    current_name = 0;
    str[0] = ' '; str[1] = 0;
    pass_empty = pe;
    mylabel = "";
  }

  ~ezexample() {
    if (dat.begin != NULL)
      free (dat.begin);
    if (current_ns.begin != NULL)
      free (current_ns.begin);
  }

  void addns(char c) {
    str[0] = c;
    VW::feature_space fs = {c, NULL, 0};
    push(dat, fs );
    current_ns.begin=NULL;
    current_ns.end=NULL;
    current_ns.end_array=NULL;
    past_seeds.push_back(current_seed);
    current_seed = VW::hash_space(*vw_ref, str);
  }

  void remns() { 
    if (dat.index() == 0) {
      current_seed = 0;
      current_ns.begin = NULL;
      current_ns.end = NULL;
      current_ns.end_array = NULL;
    } else {
      current_seed = past_seeds.back();
      past_seeds.pop_back();
      dat.end--;
      current_ns = dat->back().fs;
    }
  }

  inline fid hash(string fstr) { 
    return VW::hash_feature(*vw_ref, fstr, current_seed); 
  }
  inline fid hash(char* fstr) { 
    return VW::hash_feature_cstr(*vw_ref, fstr, current_seed);
  }
  inline fid hash(char c, string fstr) { 
    str[0] = c;
    return VW::hash_feature(*vw_ref, fstr, VW::hash_space(*vw_ref, str)); 
  }
  inline fid hash(char c, char* fstr) { 
    str[0] = c;
    return VW::hash_feature_cstr(*vw_ref, fstr, VW::hash_space(*vw_ref, str)); 
  }

  inline fid addf(fid fint, float val) {
    if (!current_ns) return 0;
    feature f = { val, fint };
    current_ns->push_back(f);
    return fint;
  }
  inline fid addf(fid    fint           ) { return addf(fint      , 1.0); }
  inline fid addf(string fstr, float val) { return addf(hash(fstr), val); }
  inline fid addf(string fstr           ) { return addf(hash(fstr), 1.0); }

  float predict() {
    static example* empty_example = VW::read_example(*vw_ref, (char*)"| ");
    example *ec = VW::import_example(*vw_ref, dat, dat_len);

    if (mylabel.length() > 0)
      VW::parse_example_label(*vw_ref, *ec, mylabel);

    vw_ref->learn(vw_ref, ec);
    if (pass_empty)
      vw_ref->learn(vw_ref, empty_example);
    float pred = ec->final_prediction;
    VW::finish_example(*vw_ref, ec);
    return pred;
  }

  inline ezexample& set_label(string label) { mylabel = label; return *this; }
  inline ezexample& operator()(fid         fint           ) { addf(fint, 1.0); return *this; }
  inline ezexample& operator()(string      fstr           ) { addf(fstr, 1.0); return *this; }
  inline ezexample& operator()(const char* fstr           ) { addf(fstr, 1.0); return *this; }
  inline ezexample& operator()(fid         fint, float val) { addf(fint, val); return *this; }
  inline ezexample& operator()(string      fstr, float val) { addf(fstr, val); return *this; }
  inline ezexample& operator()(const char* fstr, float val) { addf(fstr, val); return *this; }
  inline ezexample& operator()(const vw_namespace&n) { addns(n.namespace_letter); return *this; }
  inline ezexample& operator--() { remns(); return *this; }
  inline float      operator()() { return predict(); }


  void print() {
    cerr << "ezexample dat->size=" << dat->size() << ", current_seed=" << current_seed << endl;
    for (size_t i=0; i<dat->size(); i++) {
      cerr << "  namespace(" << dat->at(i).name << "):" << endl;
      for (size_t j=0; j<dat->at(i).second.size(); j++) {
        cerr << "    " << dat->at(i).fs[j].weight_index << "\t: " << dat->at(i).fs[j].x << endl;
      }
    }
  }
};




#endif
