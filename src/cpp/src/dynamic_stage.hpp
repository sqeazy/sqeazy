#ifndef _DYNAMIC_STAGE_H_
#define _DYNAMIC_STAGE_H_

#include <typeinfo>
#include <iostream>
#include <memory>
#include <vector>
#include <numeric>
#include <algorithm>
#include <sstream>

namespace sqeazy{

  template <typename raw_t>
  struct stage
  {

    typedef raw_t in_type;

    std::uint32_t n_threads_;

    stage():
      n_threads_(1){}

    virtual ~stage() {}

    virtual std::string input_type() const {
      return typeid(raw_t).name();
    };

    virtual std::string output_type() const = 0;
    virtual std::string name() const = 0;
    virtual std::string config() const = 0;
    virtual bool is_compressor() const = 0;

    virtual std::intmax_t max_encoded_size(std::intmax_t _incoming_size_byte) const = 0;

    virtual std::uint32_t n_threads() const {
      return n_threads_;
    }

    virtual void set_n_threads(const std::uint32_t& _number)  {
      n_threads_ = _number;
    }

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




};

#endif /* _DYNAMIC_STAGE_H_ */
