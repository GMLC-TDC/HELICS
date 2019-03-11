function v = helics_core_type_zmq_test()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 8);
  end
  v = vInitialized;
end
