/*

Copyright (C) 2017, Battelle Memorial Institute
All rights reserved.

This software was co-developed by Pacific Northwest National Laboratory, operated by the Battelle Memorial
Institute; the National Renewable Energy Laboratory, operated by the Alliance for Sustainable Energy, LLC; and the
Lawrence Livermore National Laboratory, operated by Lawrence Livermore National Security, LLC.

*/
#ifndef ACTION_MESSAGE_H_
#define ACTION_MESSAGE_H_
#pragma once

#include "core-types.h"
#include "core.h"
#include "ActionMessageDefintions.h"
#include <cereal/types/memory.hpp>
#include <memory>
#include <string>

namespace helics
{
const int32_t cmd_info_basis = 65536;

class ActionMessage
{
  public:
    // class for containing possibly larger additional information for certain message types
    class AdditionalInfo
    {
      public:
        Time Te = timeZero;  //!< event time
        Time Tdemin = timeZero;  //!< min dependent event time;
        std::string source;  //!< name of a registration
        std::string &type;  //!< alias source to type for registration
        std::string target;  //!< target or destination
        std::string &units;  //!< alias type to target for registration
        std::string orig_source;  //!< the original source
		std::string &type_out;  //!< alias type_out to orig_source for filter
		/** constructor*/
        AdditionalInfo () noexcept : type (source), units (target),type_out(orig_source){};
		/** copy constructor*/
        AdditionalInfo (const AdditionalInfo &ai)
            : Te (ai.Te), Tdemin (ai.Tdemin), source (ai.source), type (source), target (ai.target),
              units (target), orig_source (ai.orig_source), type_out(orig_source) {};
		/** move constructor*/
        AdditionalInfo (AdditionalInfo &&ai) noexcept
            : Te (ai.Te), Tdemin (ai.Tdemin), source (std::move (ai.source)), type (source),
              target (std::move (ai.target)), units (target), orig_source (std::move (ai.orig_source)), type_out(orig_source) {};
        template <class Archive>
        void save (Archive &ar) const
        {
            auto Tebase = Te.getBaseTimeCode ();
            auto Tdeminbase = Tdemin.getBaseTimeCode ();
			ar(Tebase, Tdeminbase);
			ar(source, target, orig_source);
        }

        template <class Archive>
        void load (Archive &ar)
        {
            decltype (Te.getBaseTimeCode ()) Tebase;
			decltype (Tdemin.getBaseTimeCode()) Tdeminbase;
			ar(Tebase, Tdeminbase);
            Te.setBaseTimeCode (Tebase);
            Tdemin.setBaseTimeCode (Tdeminbase);
			ar(source, target, orig_source);
        }
    };
   
    // need to try to make sure this object is under 64 bytes in size to fit in cache lines NOT there yet
  private:
    action_message_def::action_t action_ = CMD_IGNORE;  // 4 -- command
  public:
    int32_t source_id = 0;  // 8 -- for federate_id or route_id
    int32_t source_handle = 0;  // 12 -- for local handle or local code
    int32_t dest_id = 0;  // 16 fed_id for a targeted message
    int32_t dest_handle = 0;  // 20 local handle for a targetted message
	int32_t &index;			//alias to dest_handle 
    bool iterationComplete = false;  // 24 indicator that iteration has been completed
	bool &processingComplete;  //Alias to iterationComplete indictator that processing has been completed
    bool required = false;  //!< flag indicating a publication is required
    bool error = false;  //!< flag indicating an error condition associated with the command
    bool flag = false;  //!< general flag for many purposes
    Time actionTime = timeZero;  //!< the time an action took place or will take place	//32
    std::string payload;  //!< string containing the data	//64 std::string is 32 bytes on most platforms (except libc++)
    std::string &name;  //!<alias payload to a name reference for registration functions
  private:
    std::unique_ptr<AdditionalInfo> info_;  //!< pointer to an additional info structure with more data if required
  public:
    /** default constructor*/
    ActionMessage () noexcept : index(dest_handle), processingComplete(iterationComplete), name (payload) {};
    /** construct from an action type*/
    ActionMessage (action_message_def::action_t startingAction);
    /** move constructor*/
    ActionMessage (ActionMessage &&act) noexcept;
    /** build an action message from a message*/
    ActionMessage (std::unique_ptr<Message> message);
	/** construct from a string*/
	explicit ActionMessage(const std::string &bytes);
	/** construct from a data vector*/
	explicit ActionMessage(const std::vector<char> &bytes);
	/** construct from a data pointer and size*/
	explicit ActionMessage(const char *data, size_t size);
    /** destructor*/
    ~ActionMessage ();
    /** copy constructor*/
    ActionMessage (const ActionMessage &act);
    /** copy operator*/
    ActionMessage &operator= (const ActionMessage &);
    /** move assignment*/
    ActionMessage &operator= (ActionMessage &&act) noexcept;
    /** get the action of the message*/
    action_message_def::action_t action () const noexcept { return action_; }
    /** set the action*/
    void setAction (action_message_def::action_t newAction);
    /** get a reference to the additional info structure*/
    AdditionalInfo &info ();
    /** get a const ref to the info structure*/
    const AdditionalInfo &info () const;
	/** move a message data into the actionMessage
	@details take ownership of the message and move the contents out then destroy the message shell
	@param message the message to move.  
	*/
    void moveInfo (std::unique_ptr<Message> message);

    template <class Archive>
    void save (Archive &ar) const
    {
        ar (action_, source_id, source_handle, dest_id, dest_handle);
        ar (iterationComplete, required, error, flag);

        auto btc = actionTime.getBaseTimeCode ();
        ar (btc, payload);
        if (hasInfo(action_))
        {
            ar (info_);
        }
    }

    template <class Archive>
    void load (Archive &ar)
    {
        ar (action_, source_id, source_handle, dest_id, dest_handle);

        ar (iterationComplete, required, error, flag);

        decltype (actionTime.getBaseTimeCode ()) btc;
        ar (btc, payload);
        actionTime.setBaseTimeCode (btc);
        if (hasInfo(action_))
        {
            if (!info_)
            {
                info_ = std::make_unique<AdditionalInfo> ();
            }
            ar (info_);
        }
    }

    /** functions that convert to and from a byte stream*/
    void toByteArray (char *data, size_t buffer_size) const;
    void to_string (std::string &data) const;
	std::string to_string() const;
	void to_vector(std::vector<char> &data) const;
	std::vector<char> to_vector() const;
    void fromByteArray (const char *data, size_t buffer_size);
    void from_string (const std::string &data);
	void from_vector(const std::vector<char> &data);

};




/** create a new message object that copies all the information from the cmd into newly allocated memory for the
 * message
 */
std::unique_ptr<Message> createMessage (const ActionMessage &cmd);

/** create a new message object that moves all the information from the cmd into newly allocated memory for the
 * message
 */
std::unique_ptr<Message> createMessage (ActionMessage &&cmd);

bool isPriorityCommand (const ActionMessage &command);


std::string prettyPrintString(const ActionMessage &command);

std::ostream& operator<<(std::ostream& os, const ActionMessage & command);

}  // namespace helics
#endif
