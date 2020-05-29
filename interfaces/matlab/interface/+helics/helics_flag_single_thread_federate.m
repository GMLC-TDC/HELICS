function v = helics_flag_single_thread_federate()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 38);
  end
  v = vInitialized;
end
