function v = helics_core_type_zmq_test()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812630);
  end
  v = vInitialized;
end
