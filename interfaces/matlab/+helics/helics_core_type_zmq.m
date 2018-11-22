function v = helics_core_type_zmq()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1946183033);
  end
  v = vInitialized;
end
