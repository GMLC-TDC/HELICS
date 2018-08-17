function v = helics_delay_filter()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1818783863);
  end
  v = vInitialized;
end
