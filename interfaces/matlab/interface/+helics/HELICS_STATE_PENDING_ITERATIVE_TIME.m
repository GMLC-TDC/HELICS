function v = HELICS_STATE_PENDING_ITERATIVE_TIME()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 142);
  end
  v = vInitialized;
end
