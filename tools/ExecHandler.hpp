#pragma once

#include "../constant.h"

namespace wws
{
    template<abc::HandlerCode C,typename H>
    struct EH{
        static constexpr abc::HandlerCode Code = C;
        using HAND_Ty = H;
        using HAND_ARGS_Ty =  typename HAND_Ty::Args_Ty;
        
        template<typename Connect,typename Clients,typename Client,typename Data>
        static void exec(Connect &conn,Clients& clients, Client client,Data data)
        {
            HAND_ARGS_Ty args;
            create_args<0>(conn,clients,Client,args);
            H(args).handle(std::move(data));
        }

        template <size_t I,typename Tup,typename Connect,typename Clients,typename Client>
        void create_args(Connect &conn,Clients& clients, Client& client,Tup& args)
        {
            using Ty = std::remove_reference_t<decltype(std::get<I>(args))>;
            if constexpr(std::is_same_v<std::reference_wrapper<Connect>,Ty>)
            {
                std::get<I>(args) = std::ref(conn);
            }else
            if constexpr(std::is_same_v<std::reference_wrapper<Clients>,Ty>){
                std::get<I>(args) = std::ref(clients);
            }else
            if constexpr(std::is_same_v<std::reference_wrapper<Client>,Ty>){
                std::get<I>(args) = std::ref(client);
            }else
            if constexpr(std::is_same_v<Client,Ty>){
                std::get<I>(args) = std::move(client);
            }
            if constexpr ( I + 1 < std::tuple_size_v<Tup> )
            {
                create_args<I + 1>(conn,clients,Client,args);
            }
        }
    };
    template <typename Connect,typename Clients,typename Client,typename Data,typename Fir,typename ...Hs>
    void exec_handler(abc::HandlerCode c,Connect &conn,Clients& clients, Client client,Data data)
    {
        if constexpr( c == Hs::Code )
        {
            Hs::exec(conn,clients,client,std::move(data));
            return;
        }else if(sizeof...(Hs) > 0)
        {
            exec_handler<Connect,Clients,Client,Data,Hs...>(c,conn,clients,client,std::move(data));
        }
        
    }
} // namespace wws
