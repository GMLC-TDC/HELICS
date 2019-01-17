function v = helics_core_type_zmq()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1464812623);
  end
  v = vInitialized;
end
