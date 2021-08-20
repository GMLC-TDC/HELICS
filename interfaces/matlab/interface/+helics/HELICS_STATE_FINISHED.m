function v = HELICS_STATE_FINISHED()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 144);
  end
  v = vInitialized;
end
