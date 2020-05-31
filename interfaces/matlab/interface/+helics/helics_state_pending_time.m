function v = helics_state_pending_time()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 124);
  end
  v = vInitialized;
end
