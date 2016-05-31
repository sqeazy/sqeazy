#ifndef _DYNAMIC_STAGE_H_
#define _DYNAMIC_STAGE_H_

#include <typeinfo>
#include <iostream>
#include <memory>
#include <vector>
#include <numeric>

namespace sqeazy{

  template <typename raw_t>
  struct stage
  {

    typedef raw_t in_type;
    virtual ~stage() {}
    
    virtual std::string input_type() const {
      return typeid(raw_t).name();
    };
    
    virtual std::string output_type() const = 0;
    virtual std::string name() const = 0;
    virtual std::string config() const = 0;
    virtual bool is_compressor() const = 0;

    virtual std::intmax_t max_encoded_size(std::intmax_t _incoming_size_byte) const = 0;
    
  };
  
  /**
     \brief object that can filter the contents of incoming data, but will not play with their size in memory
     
     \param[in] 
     
     \return 
     \retval 
     
  */
  template <typename raw_t>
  struct filter : public stage<raw_t>
  {

    constexpr static bool is_compressor = false;
    typedef raw_t in_type;
    typedef raw_t out_type;

    filter(const std::string& _params = ""){}

    template <typename T>
    filter(const filter<T>& _rhs):
      stage<raw_t>::stage()
    {}
    
    virtual ~filter() {}

    virtual out_type* encode(const in_type*, out_type*,const std::vector<std::size_t>&) {return nullptr;};
    virtual out_type* encode(const in_type* _in, out_type* _out,std::size_t len) {

      std::vector<std::size_t> shape(1,len);
      return encode(_in,_out,shape);

    };

    
    virtual int decode(const in_type*,
		       out_type*,
		       const std::vector<std::size_t>&,
		       std::vector<std::size_t>) const {return 1;};
    
    virtual int decode(const in_type* _in, out_type* _out,
		       std::size_t len,
		       std::size_t) const {

      std::vector<std::size_t> shape(1,len);
      return decode(_in,_out,shape,shape);

    };
    
    virtual std::intmax_t max_encoded_size(std::intmax_t _incoming_size_byte) const override { return 0; };
  };

  /**
     \brief object that can alter the contents of incoming data, it will potentially play with their size
     
     \param[in] 
     
     \return 
     \retval 
     
  */
  template <typename raw_t, typename compressed_t = char>
  struct sink : public stage<raw_t>
  {

    constexpr static bool is_compressor = true;
    typedef raw_t in_type;
    typedef compressed_t out_type;
    
    virtual ~sink() {}
    sink(const std::string& _params = ""){}
    
    /**
       \brief encode raw_t buffer _in of shape len and write results to _out
       
       \param[in] 
       
       \return out_type* pointer to the last+1 element of _out that was written (if error occurred nullptr is returned)
       \retval 
       
    */
    virtual out_type* encode(const raw_t* _in, out_type* _out,std::size_t len) {

      std::vector<std::size_t> shape(1,len);
      return encode(_in,_out,shape);

    };
    virtual out_type* encode(const raw_t*, out_type*,const std::vector<std::size_t>&) {return nullptr;};

    virtual int decode(const out_type* _in,
		       raw_t* _out,
		       std::size_t inlen,
		       std::size_t outlen = 0) const {

      std::vector<std::size_t> inshape(1,inlen);
      std::vector<std::size_t> outshape(1,outlen);
      return decode(_in,_out,inshape,outshape);

    };
    virtual int decode(const out_type* _in, raw_t* _out,
		       const std::vector<std::size_t>& _inshape,
		       std::vector<std::size_t> _outshape = std::vector<std::size_t>()) const {return 1;};

    virtual std::intmax_t max_encoded_size(std::intmax_t _incoming_size_byte) const override { return 0; };
  };    
    


  template <typename T>
  const std::shared_ptr<stage<typename T::in_type> > const_stage_view(const std::shared_ptr<T>& _in){
  // const stage<typename T::in_type>* const_stage_view(const std::shared_ptr<T>& _in){

    using return_t = stage<typename T::in_type>;
    
    auto value = std::dynamic_pointer_cast<return_t>(_in);// using return_t = const stage<typename T::in_type>*;
    
    // auto value = dynamic_cast<return_t>(_in.get());
    
    if(value)
      return value;
    else
      return nullptr;
  }

  template <typename T>
  std::shared_ptr<stage<typename T::in_type> > stage_view(std::shared_ptr<T>& _in){

    using return_t = stage<typename T::in_type>;
    
    auto value = std::dynamic_pointer_cast<return_t>(_in);
    
    if(value)
      return value;
    else
      return nullptr;
  }

  template <typename T = void>
  struct blank_filter : public  filter<T>
  {

    typedef void raw_t;
    ~blank_filter() {}
    blank_filter(const std::string& _params="") {}
    
    std::string input_type() const {return "";}
    std::string output_type() const {return "";}
    std::string name() const { return "blank";}
    std::string config() const { return "";}

    bool is_compressor() const {return false;}

    raw_t* encode(const raw_t* _in, raw_t* _out,std::vector<std::size_t> _shape) override final { return nullptr; };
    int decode(const raw_t* _in, raw_t* _out,std::vector<std::size_t> _shape) const override final { return 1; };
    
  };
  
  template <typename T = void>
  struct blank_sink : public  sink<T,void>
  {

    typedef T raw_t;
    typedef typename sink<void>::out_type compressed_t;
    ~blank_sink() {}
    blank_sink(const std::string& _params="") {}
    
    std::string input_type() const {return "";}
    std::string output_type() const {return "";}
    std::string name() const { return "blank";}
    std::string config() const { return "";}
    
    bool is_compressor() const {return false;}

    compressed_t* encode(const raw_t* _in, compressed_t* _out,std::vector<std::size_t> _shape) override final { return nullptr; };
    int decode(const compressed_t* _in,
	       raw_t* _out,
	       std::vector<std::size_t> _inshape,
	       std::vector<std::size_t> _outshape = std::vector<std::size_t>()) const override final { return 1; };
    
  };


  template <typename filter_t>
  struct stage_chain {


    typedef typename filter_t::in_type incoming_t;
    typedef typename filter_t::out_type outgoing_t;

    typedef filter_t			filter_base_t;
    typedef std::shared_ptr< filter_t > filter_ptr_t;
    typedef std::vector<filter_ptr_t>   filter_holder_t;

    filter_holder_t chain_;

    friend void swap(stage_chain & _lhs, stage_chain & _rhs){

      std::swap(_lhs.chain_,_rhs.chain_);
    }
    
    stage_chain():
      chain_(){}

    stage_chain(std::initializer_list<filter_ptr_t> _chain):
      chain_(){

      for(const filter_ptr_t& step : _chain)
	{
	  chain_.push_back(step);
	}

    }


    stage_chain(const stage_chain& _rhs):
      chain_(_rhs.chain_.begin(), _rhs.chain_.end())
    {}

    
    stage_chain& operator=(stage_chain _rhs){

      swap(*this, _rhs);

      return *this;
    }

    const std::size_t size() const { return chain_.size(); }
    const bool empty() const { return chain_.empty(); }
    void clear() { chain_.clear(); };
    
    template <typename pointee_t>
    void push_back(const std::shared_ptr<pointee_t>& _new){

      if(std::is_base_of<filter_t,pointee_t>::value){
	auto casted = std::dynamic_pointer_cast<filter_t>(_new);
	chain_.push_back(casted);
      }
      
    }

    void add(const filter_ptr_t& _new){

      this->push_back(_new);

    }

    bool is_compressor() const {
      
      return filter_t::is_compressor ;
      
    }
    
    bool valid() const {

      bool value = true;
      if(chain_.empty())
        return !value;

      if(chain_.size()==1 || std::is_same<filter_t,filter<incoming_t> >::value)
	return true;

      //TODO: this loop is obsolete for sqy::filter types
      for(unsigned i = 1; i < chain_.size(); ++i)
	{
	  auto this_filter =  const_stage_view(chain_[i]);
	  auto last_filter =  const_stage_view(chain_[i-1]);

	  if(!this_filter || !last_filter){
	    value = false ;
	    break;
	  }
	  value = value && (this_filter->input_type() == last_filter->output_type());
	}

      return value;
      
    }

    filter_ptr_t& front() {

      return chain_.front();
      
    }

    const filter_ptr_t& front() const {

      return chain_.front();
      
    }

    filter_ptr_t& back() {

      return chain_.back();
      
    }

    const filter_ptr_t& back() const {

      return chain_.back();
      
    }

    std::string name() const
    {

      std::ostringstream value;

      //TODO: REFACTOR THIS!
      for(const auto& step : chain_)
	{
	  auto cview = const_stage_view(step);

	  if(!cview){
	    std::cerr << "[src/cpp/src/dynamic_stage.hpp] \t unable to parse chain step\n";
	    continue;
	  }
	    
	  value << cview->name();
	  std::string cfg = cview->config();

	  if(!cfg.empty())
	    value << "(" << cfg << ")";
		
	  if(step != chain_.back())
	    value << "->";
	}

      return value.str();
    }

    std::string config() const
    {
      return name();
    }

    /**
       \brief encode one-dimensional array _in and write results to _out
       
       \param[in] 
       
       \return 
       \retval 
       
    */
    
    outgoing_t * encode(const incoming_t *_in, outgoing_t *_out, std::size_t _len) /*override final*/ {

      std::vector<std::size_t> shape(1,_len);
      return encode(_in,_out,shape);

    }

    
    /**
       \brief encode one-dimensional array _in and write results to _out
       
       \param[in] _in input buffer
       \param[out] _out output buffer must be at least of size max_encoded_size
       \param[in] _shape shape in input buffer in nDim

       \return 
       \retval 
       
    */
    
    outgoing_t* encode(const incoming_t *_in,
		       outgoing_t *_out,
		       const std::vector<std::size_t>& _shape) /*override final*/ {
      outgoing_t* value = nullptr;
      std::size_t len = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
      //      std::size_t max_len_byte = len*sizeof(incoming_t);

      std::vector<incoming_t> temp_in(_in, _in+len);
      std::vector<outgoing_t> temp_out(temp_in.size());
      std::size_t compressed_items = 0;
      
      for( std::size_t fidx = 0;fidx<chain_.size();++fidx )
	{
	  
	  auto encoded_end = chain_[fidx]->encode( temp_in.data(),
						   temp_out.data(),
						  _shape);

	  compressed_items = encoded_end - temp_out.data();
	  if(compressed_items>temp_out.size()){
	    std::ostringstream msg;
	    msg << __FILE__ << ":" << __LINE__ << "\t encode wrote past the end of temporary buffers\n";
	    throw std::runtime_error(msg.str());
	  }
	  
	  value = reinterpret_cast<decltype(value)>(encoded_end);	    
	  std::copy(temp_out.data(),value,temp_in.begin());
	}

      std::size_t outgoing_size = value-((decltype(value))&temp_out[0]);
      std::copy(temp_out.data(),value,
		_out);
      value = _out + outgoing_size;     
      return value;
      
    }


    int decode(const outgoing_t *_in, incoming_t *_out, std::size_t _len) const /*override final*/ {

      std::vector<std::size_t> shape(1,_len);
      return decode(_in,_out,shape);
      
    }
    /**
       \brief decode one-dimensional array _in and write results to _out
       
       \param[in] _in input buffer
       \param[out] _out output buffer //NOTE: might be larger than _in for sink type pipelines
       \param[in] _shape of input buffer size in units of its type, aka outgoing_t
       
       \return 
       \retval 
       
    */
    
    int decode(const outgoing_t *_in,
	       incoming_t *_out,
	       const std::vector<std::size_t>& _ishape,
	       std::vector<std::size_t> _oshape = std::vector<std::size_t>()) const /*override final*/ {
      
      int value = 0;
      int err_code = 0;
      if(_oshape.empty())
	_oshape = _ishape;
      
      std::size_t len = std::accumulate(_ishape.begin(), _ishape.end(),1,std::multiplies<std::size_t>());
      std::vector<incoming_t> temp(len,0);
      std::copy(_in,_in+len,temp.begin());

      auto rev_begin = chain_.rbegin();
      auto rev_end   = chain_.rend();
      int fidx = 0;
      // for( int fidx = (chain_.size()-1);fidx>=0;--fidx )
      for(;rev_begin!=rev_end;++rev_begin,++fidx)
	{
	    
	  err_code = (*rev_begin)->decode(temp.data(),
					  _out,
					  _ishape,
					  _oshape);
	  value += err_code ? (10*(fidx+1))+err_code : 0;
	  std::copy(_out,
		    _out+len,
		    temp.data());
	}

      return value;

    }

    std::intmax_t max_encoded_size(std::intmax_t _incoming_size_byte) const { 

      std::vector<std::intmax_t> values;
      for( auto & f : chain_ )
	values.push_back(f->max_encoded_size(_incoming_size_byte));

      if(values.empty())
	return 0;
      else
	return *std::max_element(values.begin(), values.end());

	// if(!is_compressor())
      // 	return _incoming_size_byte;
      // else
      // 	return chain_.front()->max_encoded_size(_incoming_size_byte);
      
    }
  };

  
};

#endif /* _DYNAMIC_STAGE_H_ */
