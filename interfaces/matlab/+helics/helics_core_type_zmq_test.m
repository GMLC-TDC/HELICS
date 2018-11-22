function v = helics_core_type_zmq_test()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183040);
  end
  v = vInitialized;
end
