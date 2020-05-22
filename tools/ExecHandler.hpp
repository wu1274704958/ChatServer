#pragma once

#include "../constant.h"

namespace wws
{
    template<abc::HandlerCode C,typename H>
    struct EH{
        static constexpr abc::HandlerCode Code = C;
        using HAND_Ty = H;
        using HAND_ARGS_Ty =  typename HAND_Ty::Args_Ty;
        
        template<typename Tup,typename Data>
        static void exec(Tup& tup,Data data)
        {
            H(look_need<HAND_ARGS_Ty,0,Tup>(tup)).handle(std::move(data));
        }

        template <size_t I,typename Tup,typename T>
        static std::tuple<T> create_args(Tup& tup)
        {
            using Ty = std::tuple_element_t<I, Tup>;
            if constexpr(std::is_same_v<T, Ty>)
            {
                return std::make_tuple(std::get<I>(tup));
            }else
            if constexpr(std::is_same_v<std::reference_wrapper<T>,Ty>){
                return std::make_tuple(std::get<I>(tup).get());
            }else
            if constexpr(std::is_same_v<std::shared_ptr<T>,Ty>){
                return std::make_tuple(*(std::get<I>(tup)));
            }else
            if constexpr(std::is_same_v<T, std::reference_wrapper<Ty>>){
                return std::make_tuple(std::ref(std::get<I>(tup)));
            }else
            if constexpr ( I + 1 < std::tuple_size_v<Tup> )
            {
                return create_args<I + 1,Tup,T>(tup);
            }
            else {
                return std::make_tuple(std::get<I>(tup));
            }
        }

        template <typename Tup,size_t I,typename Tup2>
        static decltype(auto) look_need(Tup2& tup2)
        {
            if constexpr (I < std::tuple_size_v<Tup>)
            {
                auto ret = create_args < 0, Tup2, std::tuple_element_t<I,Tup>>(tup2);
                if constexpr (I + 1 < std::tuple_size_v<Tup>)
                {
                    return std::tuple_cat( ret,look_need<Tup,I + 1, Tup2>(tup2));
                }
                else {
                    return ret;
                }
            }
        }

    };
    template <typename Tup,typename Data,typename Fir,typename ...Hs>
    void exec_handler(abc::HandlerCode c, Tup tup,Data data)
    {
        if (c == Fir::template Code )
        {
            Fir::template exec(tup,std::move(data));
            return;
        } 
        if constexpr(sizeof...(Hs) > 0)
        {
            exec_handler<Tup,Data,Hs...>(tup,std::move(data));
        }
        
    }
} // namespace wws
