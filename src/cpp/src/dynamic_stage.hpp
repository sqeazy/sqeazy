#ifndef _DYNAMIC_STAGE_H_
#define _DYNAMIC_STAGE_H_

#include <typeinfo>
#include <memory>
#include <vector>

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
    
    virtual ~filter() {}

    virtual out_type* encode(const in_type*, out_type*,std::vector<std::size_t>) {return nullptr;};
    virtual out_type* encode(const in_type* _in, out_type* _out,std::size_t len) {

      std::vector<std::size_t> shape(1,len);
      return encode(_in,_out,shape);

    };


    virtual int decode(const in_type*, out_type*,std::vector<std::size_t>) const {return 1;};
    virtual int decode(const in_type* _in, out_type* _out,std::size_t len) const {

      std::vector<std::size_t> shape(1,len);
      return decode(_in,_out,shape);

    };
    
    virtual std::intmax_t max_encoded_size(std::intmax_t _incoming_size_byte) const { return 0; };
  };

  /**
     \brief object that can alter the contents of incoming data, it will potentially play with their size
     
     \param[in] 
     
     \return 
     \retval 
     
  */
  template <typename raw_t, typename compressed_t = std::int8_t>
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
    virtual out_type* encode(const raw_t*, out_type*,std::vector<std::size_t>) {return nullptr;};

    virtual int decode(const out_type* _in, raw_t* _out,std::size_t len) const {

      std::vector<std::size_t> shape(1,len);
      return decode(_in,_out,shape);

    };
    virtual int decode(const out_type*, raw_t*,std::vector<std::size_t>) const {return 1;};

    virtual std::intmax_t max_encoded_size(std::intmax_t _incoming_size_byte) const { return 0; };
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

  
  struct blank_filter : public  filter<void>
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
  
  
  struct blank_sink : public  sink<void>
  {

    typedef void raw_t;
    ~blank_sink() {}
    blank_sink(const std::string& _params="") {}
    
    std::string input_type() const {return "";}
    std::string output_type() const {return "";}
    std::string name() const { return "blank";}
    std::string config() const { return "";}
    
    bool is_compressor() const {return false;}

    std::int8_t* encode(const raw_t* _in, std::int8_t* _out,std::vector<std::size_t> _shape) override final { return nullptr; };
    int decode(const std::int8_t* _in, raw_t* _out,std::vector<std::size_t> _shape) const override final { return 1; };
    
  };


  template <typename filter_t>
  struct stage_chain {


    typedef typename filter_t::in_type incoming_t;
    typedef typename filter_t::out_type outgoing_t;
    
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
      chain_(_rhs.chain_)
    {}

    stage_chain& operator=(stage_chain _rhs){

      swap(*this, _rhs);

      return *this;
    }

    const std::size_t size() const { return chain_.size(); }
    const bool empty() const { return chain_.empty(); }

    void push_back(const filter_ptr_t& _new){

      chain_.push_back(_new);
      
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
	  value << const_stage_view(step)->name();
	  std::string cfg = const_stage_view(step)->config();

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
    
    outgoing_t* encode(const incoming_t *_in, outgoing_t *_out, std::vector<std::size_t> _shape) /*override final*/ {
      outgoing_t* value = nullptr;
      std::size_t len = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
      //      std::size_t max_len_byte = len*sizeof(incoming_t);

      std::vector<incoming_t> temp_in(_in, _in+len);
      std::vector<outgoing_t> temp_out(temp_in.size());

      for( std::size_t fidx = 0;fidx<chain_.size();++fidx )
	{
	  
	  auto encoded_end = chain_[fidx]->encode(&temp_in[0],
						  &temp_out[0],
						  _shape);
	  value = reinterpret_cast<decltype(value)>(encoded_end);
	  std::copy(&temp_out[0],value,temp_in.begin());
	}

      std::size_t outgoing_size = value-((decltype(value))&temp_out[0]);
      std::copy(&temp_out[0],value, _out);
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
    
    int decode(const outgoing_t *_in, incoming_t *_out, std::vector<std::size_t> _shape) const /*override final*/ {
      int value = 0;
      int err_code = 0;
      std::size_t len = std::accumulate(_shape.begin(), _shape.end(),1,std::multiplies<std::size_t>());
      std::vector<incoming_t> temp(len,0);
      std::copy(_in,_in+len,temp.begin());

      for( std::size_t fidx = 0;fidx<chain_.size();++fidx )
	{
	    
	  err_code = chain_[fidx]->decode(&temp[0],
					  _out,
					  _shape);
	  value += err_code ? (10*(fidx+1))+err_code : 0;
	  std::copy(_out, _out+len,temp.begin());
	}

      return value;

    }

    std::intmax_t max_encoded_size(std::intmax_t _incoming_size_byte){
      if(!is_compressor())
	return _incoming_size_byte;
      else
	return chain_.front()->max_encoded_size(_incoming_size_byte);
    }
  };

  
};

#endif /* _DYNAMIC_STAGE_H_ */
