if(MSVC)

  set(
    doc_sources
    apps/Echo.md
    apps/Broker.md
    apps/App.md
    apps/Player.md
    apps/index.md
    apps/Source.md
    configuration/CoreTypes.md
    configuration/Federate.md
    configuration/index.md
    configuration/Timing.md
    configuration/Filters.md
    configuration/MessageFederate.md
    configuration/ValueFederate.md
  )

  add_custom_target(helics_docs SOURCES ${doc_sources})

endif(MSVC)
