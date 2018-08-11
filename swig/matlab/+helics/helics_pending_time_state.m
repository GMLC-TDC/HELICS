function v = helics_pending_time_state()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 24);
  end
  v = vInitialized;
end
